#pragma once

#include <atomic>
#include <chrono>
#include <map>
#include <mutex>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>

namespace metrics {

/**
 * @brief Простая реализация Prometheus Histogram для квантилей
 * 
 * Бакеты: 0.005, 0.01, 0.025, 0.05, 0.1, 0.25, 0.5, 1.0, 2.5, 5.0, 10.0
 * 
 * Prometheus рассчитывает квантили на основе бакетов с помощью
 * histogram_quantile() функции.
 */
class Histogram {
public:
    // Стандартные бакеты для HTTP latency (в секундах)
    Histogram() : buckets_({0.005, 0.01, 0.025, 0.05, 0.1, 0.25, 0.5, 1.0, 2.5, 5.0, 10.0}) {
        counts_.resize(buckets_.size() + 1, 0);  // +1 для +Inf
    }
    
    void observe(double value) {
        std::lock_guard<std::mutex> lock(mutex_);
        sum_ += value;
        count_++;
        
        // Находим бакет
        for (size_t i = 0; i < buckets_.size(); ++i) {
            if (value <= buckets_[i]) {
                counts_[i]++;
            }
        }
        // +Inf бакет (последний) всегда увеличивается
        counts_.back()++;
        
        // Отслеживаем max
        if (value > max_) {
            max_ = value;
        }
    }
    
    std::string serialize(const std::string& name, 
                         const std::map<std::string, std::string>& labels = {}) const {
        std::lock_guard<std::mutex> lock(mutex_);
        std::ostringstream oss;
        
        std::string labelStr = formatLabels(labels);
        
        // Бакеты
        uint64_t cumulative = 0;
        for (size_t i = 0; i < buckets_.size(); ++i) {
            cumulative += counts_[i];
            oss << name << "_bucket{" << labelStr;
            if (!labelStr.empty()) oss << ",";
            oss << "le=\"" << buckets_[i] << "\"} " << cumulative << "\n";
        }
        // +Inf бакет
        oss << name << "_bucket{" << labelStr;
        if (!labelStr.empty()) oss << ",";
        oss << "le=\"+Inf\"} " << count_ << "\n";
        
        // Sum и count
        oss << name << "_sum{" << labelStr << "} " << sum_ << "\n";
        oss << name << "_count{" << labelStr << "} " << count_ << "\n";
        
        return oss.str();
    }
    
    double getMax() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return max_;
    }
    
    uint64_t getCount() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return count_;
    }
    
    double getSum() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return sum_;
    }

private:
    std::string formatLabels(const std::map<std::string, std::string>& labels) const {
        std::ostringstream oss;
        bool first = true;
        for (const auto& [key, value] : labels) {
            if (!first) oss << ",";
            oss << key << "=\"" << value << "\"";
            first = false;
        }
        return oss.str();
    }
    
    std::vector<double> buckets_;
    mutable std::vector<uint64_t> counts_;
    mutable uint64_t count_ = 0;
    mutable double sum_ = 0.0;
    mutable double max_ = 0.0;
    mutable std::mutex mutex_;
};

/**
 * @brief Counter метрика
 */
class Counter {
public:
    void increment(uint64_t value = 1) {
        value_.fetch_add(value, std::memory_order_relaxed);
    }
    
    uint64_t get() const {
        return value_.load(std::memory_order_relaxed);
    }
    
private:
    std::atomic<uint64_t> value_{0};
};

/**
 * @brief Gauge метрика
 */
class Gauge {
public:
    void set(double value) {
        std::lock_guard<std::mutex> lock(mutex_);
        value_ = value;
    }
    
    void increment(double value = 1.0) {
        std::lock_guard<std::mutex> lock(mutex_);
        value_ += value;
    }
    
    void decrement(double value = 1.0) {
        std::lock_guard<std::mutex> lock(mutex_);
        value_ -= value;
    }
    
    double get() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return value_;
    }
    
private:
    double value_ = 0.0;
    mutable std::mutex mutex_;
};

/**
 * @brief Ключ для метрик по методу и пути
 */
struct MetricKey {
    std::string method;
    std::string path;
    
    bool operator<(const MetricKey& other) const {
        if (method != other.method) return method < other.method;
        return path < other.path;
    }
};

/**
 * @brief Глобальный реестр метрик
 * 
 * Singleton для сбора всех метрик приложения.
 * Thread-safe.
 */
class MetricsRegistry {
public:
    static MetricsRegistry& instance() {
        static MetricsRegistry instance;
        return instance;
    }
    
    // HTTP Request Duration (Histogram для квантилей)
    void observeRequestDuration(const std::string& method, 
                                 const std::string& path, 
                                 double durationSeconds) {
        std::lock_guard<std::mutex> lock(mutex_);
        MetricKey key{method, normalizePath(path)};
        requestDurations_[key].observe(durationSeconds);
    }
    
    // HTTP Requests Total (Counter)
    void incrementRequestsTotal(const std::string& method, 
                                 const std::string& path,
                                 int statusCode) {
        std::lock_guard<std::mutex> lock(mutex_);
        MetricKey key{method, normalizePath(path)};
        std::string status = std::to_string(statusCode);
        requestsTotal_[{key, status}].increment();
    }
    
    // HTTP Errors (5xx)
    void incrementErrors(const std::string& method, const std::string& path) {
        std::lock_guard<std::mutex> lock(mutex_);
        MetricKey key{method, normalizePath(path)};
        errorsTotal_[key].increment();
    }
    
    // Uptime
    double getUptimeSeconds() const {
        auto now = std::chrono::steady_clock::now();
        return std::chrono::duration<double>(now - startTime_).count();
    }
    
    /**
     * @brief Сериализация всех метрик в Prometheus формат
     */
    std::string serialize() const {
        std::lock_guard<std::mutex> lock(mutex_);
        std::ostringstream oss;
        
        // Uptime
        oss << "# HELP app_uptime_seconds Time since application start\n";
        oss << "# TYPE app_uptime_seconds gauge\n";
        oss << "app_uptime_seconds " << getUptimeSeconds() << "\n\n";
        
        // HTTP Request Duration Histogram
        oss << "# HELP http_request_duration_seconds HTTP request latency in seconds\n";
        oss << "# TYPE http_request_duration_seconds histogram\n";
        for (const auto& [key, histogram] : requestDurations_) {
            std::map<std::string, std::string> labels = {
                {"method", key.method},
                {"path", key.path}
            };
            oss << histogram.serialize("http_request_duration_seconds", labels);
        }
        oss << "\n";
        
        // HTTP Request Duration Max (для max квантиля)
        oss << "# HELP http_request_duration_max_seconds Maximum request duration\n";
        oss << "# TYPE http_request_duration_max_seconds gauge\n";
        for (const auto& [key, histogram] : requestDurations_) {
            oss << "http_request_duration_max_seconds{method=\"" << key.method 
                << "\",path=\"" << key.path << "\"} " << histogram.getMax() << "\n";
        }
        oss << "\n";
        
        // HTTP Requests Total
        oss << "# HELP http_requests_total Total number of HTTP requests\n";
        oss << "# TYPE http_requests_total counter\n";
        for (const auto& [keyStatus, counter] : requestsTotal_) {
            const auto& [key, status] = keyStatus;
            oss << "http_requests_total{method=\"" << key.method 
                << "\",path=\"" << key.path 
                << "\",status=\"" << status << "\"} " << counter.get() << "\n";
        }
        oss << "\n";
        
        // HTTP Errors Total (5xx)
        oss << "# HELP http_errors_total Total number of HTTP 5xx errors\n";
        oss << "# TYPE http_errors_total counter\n";
        for (const auto& [key, counter] : errorsTotal_) {
            oss << "http_errors_total{method=\"" << key.method 
                << "\",path=\"" << key.path << "\"} " << counter.get() << "\n";
        }
        oss << "\n";
        
        return oss.str();
    }
    
private:
    MetricsRegistry() : startTime_(std::chrono::steady_clock::now()) {}
    
    // Нормализация пути (заменяем ID на {id})
    std::string normalizePath(const std::string& path) const {
        // Простая нормализация: /api/v1/users/123 -> /api/v1/users/{id}
        std::string result = path;
        
        // Убираем trailing slash
        if (!result.empty() && result.back() == '/') {
            result.pop_back();
        }
        
        // Ищем числовые сегменты и заменяем на {id}
        size_t pos = 0;
        while ((pos = result.find('/', pos)) != std::string::npos) {
            size_t start = pos + 1;
            size_t end = result.find('/', start);
            if (end == std::string::npos) end = result.length();
            
            std::string segment = result.substr(start, end - start);
            bool isNumeric = !segment.empty() && 
                std::all_of(segment.begin(), segment.end(), ::isdigit);
            
            if (isNumeric) {
                result.replace(start, end - start, "{id}");
            }
            
            pos = start;
        }
        
        return result;
    }
    
    std::chrono::steady_clock::time_point startTime_;
    
    // Метрики
    std::map<MetricKey, Histogram> requestDurations_;
    std::map<std::pair<MetricKey, std::string>, Counter> requestsTotal_;  // key + status
    std::map<MetricKey, Counter> errorsTotal_;
    
    mutable std::mutex mutex_;
};

/**
 * @brief RAII Timer для измерения latency
 * 
 * Использование:
 * ```cpp
 * {
 *     ScopedTimer timer("GET", "/api/v1/users");
 *     // ... обработка запроса ...
 * } // автоматически записывает время в MetricsRegistry
 * ```
 */
class ScopedTimer {
public:
    ScopedTimer(const std::string& method, const std::string& path)
        : method_(method)
        , path_(path)
        , start_(std::chrono::steady_clock::now())
    {}
    
    ~ScopedTimer() {
        auto end = std::chrono::steady_clock::now();
        double duration = std::chrono::duration<double>(end - start_).count();
        MetricsRegistry::instance().observeRequestDuration(method_, path_, duration);
    }
    
    // Получить текущую длительность (до завершения)
    double elapsed() const {
        auto now = std::chrono::steady_clock::now();
        return std::chrono::duration<double>(now - start_).count();
    }
    
private:
    std::string method_;
    std::string path_;
    std::chrono::steady_clock::time_point start_;
};

} // namespace metrics
