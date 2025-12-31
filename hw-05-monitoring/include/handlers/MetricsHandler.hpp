#pragma once

#include <IHttpHandler.hpp>
#include "metrics/PrometheusMetrics.hpp"
#include <iostream>

/**
 * @brief HTTP Handler для Prometheus метрик
 * 
 * Endpoint: GET /metrics
 * 
 * Возвращает метрики в формате Prometheus text exposition:
 * - http_request_duration_seconds (histogram) - для квантилей p50, p95, p99
 * - http_request_duration_max_seconds (gauge) - максимальное время ответа
 * - http_requests_total (counter) - общее количество запросов по методам/путям/статусам
 * - http_errors_total (counter) - количество 5xx ошибок
 * - app_uptime_seconds (gauge) - время работы приложения
 * 
 * Пример использования в Grafana:
 * - Latency p50: histogram_quantile(0.5, rate(http_request_duration_seconds_bucket[5m]))
 * - Latency p95: histogram_quantile(0.95, rate(http_request_duration_seconds_bucket[5m]))
 * - Latency p99: histogram_quantile(0.99, rate(http_request_duration_seconds_bucket[5m]))
 * - RPS: rate(http_requests_total[1m])
 * - Error Rate: rate(http_errors_total[1m])
 */
class MetricsHandler : public IHttpHandler {
public:
    MetricsHandler() {
        std::cout << "[MetricsHandler] Created" << std::endl;
    }
    
    virtual ~MetricsHandler() = default;
    
    void handle(IRequest& req, IResponse& res) override {
        std::cout << "[MetricsHandler] Handling /metrics request" << std::endl;
        
        // Получаем все метрики из реестра
        std::string metricsOutput = metrics::MetricsRegistry::instance().serialize();
        
        res.setStatus(200);
        res.setHeader("Content-Type", "text/plain; version=0.0.4; charset=utf-8");
        res.setBody(metricsOutput);
    }
};
