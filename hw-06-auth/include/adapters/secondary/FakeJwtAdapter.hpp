#pragma once

#include "ports/output/IJwtProvider.hpp"
#include <nlohmann/json.hpp>
#include <unordered_set>
#include <mutex>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <random>

namespace adapters::secondary {

/**
 * @brief Fake JWT Provider для разработки и тестирования
 * 
 * Простая реализация без криптографической подписи.
 * Формат токена: header.payload.signature
 *   - header: фиктивный
 *   - payload: base64url(JSON)
 *   - signature: "fake_signature"
 * 
 * @warning НЕ использовать в production!
 */
class FakeJwtAdapter : public ports::output::IJwtProvider {
public:
    explicit FakeJwtAdapter(int lifetimeSeconds = 86400)
        : lifetime_(lifetimeSeconds)
        , rng_(std::random_device{}())
    {}

    std::string createToken(
        const std::string& userId,
        const std::string& username
    ) override {
        auto now = std::chrono::system_clock::now();
        auto nowSec = std::chrono::duration_cast<std::chrono::seconds>(
            now.time_since_epoch()
        ).count();

        nlohmann::json payload;
        payload["jti"] = generateTokenId();
        payload["sub"] = userId;
        payload["username"] = username;
        payload["iat"] = nowSec;
        payload["exp"] = nowSec + lifetime_;

        std::string payloadStr = base64UrlEncode(payload.dump());
        
        // Фиктивный header
        const std::string header = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9";
        
        return header + "." + payloadStr + ".fake_signature";
    }

    bool validateToken(const std::string& token) override {
        if (isBlacklisted(token)) {
            return false;
        }

        auto claims = extractClaims(token);
        if (!claims) {
            return false;
        }

        auto now = std::chrono::system_clock::now();
        auto nowSec = std::chrono::duration_cast<std::chrono::seconds>(
            now.time_since_epoch()
        ).count();

        return claims->expiresAt > nowSec;
    }

    std::optional<ports::output::TokenClaims> extractClaims(
        const std::string& token
    ) override {
        try {
            // Разбиваем токен на части
            size_t firstDot = token.find('.');
            size_t secondDot = token.find('.', firstDot + 1);

            if (firstDot == std::string::npos || secondDot == std::string::npos) {
                return std::nullopt;
            }

            std::string payloadB64 = token.substr(firstDot + 1, secondDot - firstDot - 1);
            std::string payloadJson = base64UrlDecode(payloadB64);

            auto payload = nlohmann::json::parse(payloadJson);

            ports::output::TokenClaims claims;
            claims.tokenId = payload.value("jti", "");
            claims.userId = payload.value("sub", "");
            claims.username = payload.value("username", "");
            claims.issuedAt = payload.value("iat", int64_t(0));
            claims.expiresAt = payload.value("exp", int64_t(0));

            return claims;
        } catch (...) {
            return std::nullopt;
        }
    }

    void blacklistToken(const std::string& token) override {
        std::lock_guard<std::mutex> lock(mutex_);
        blacklist_.insert(token);
    }

    bool isBlacklisted(const std::string& token) override {
        std::lock_guard<std::mutex> lock(mutex_);
        return blacklist_.find(token) != blacklist_.end();
    }

    int getTokenLifetime() const override {
        return lifetime_;
    }

private:
    int lifetime_;
    std::unordered_set<std::string> blacklist_;
    std::mutex mutex_;
    std::mt19937_64 rng_;

    std::string generateTokenId() {
        std::uniform_int_distribution<uint64_t> dist;
        std::stringstream ss;
        ss << std::hex << dist(rng_);
        return ss.str();
    }

    std::string base64UrlEncode(const std::string& input) {
        static const char table[] = 
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
        
        std::string output;
        output.reserve(((input.size() + 2) / 3) * 4);

        for (size_t i = 0; i < input.size(); i += 3) {
            uint32_t n = static_cast<unsigned char>(input[i]) << 16;
            
            if (i + 1 < input.size()) {
                n |= static_cast<unsigned char>(input[i + 1]) << 8;
            }
            if (i + 2 < input.size()) {
                n |= static_cast<unsigned char>(input[i + 2]);
            }

            output.push_back(table[(n >> 18) & 0x3F]);
            output.push_back(table[(n >> 12) & 0x3F]);
            
            if (i + 1 < input.size()) {
                output.push_back(table[(n >> 6) & 0x3F]);
            }
            if (i + 2 < input.size()) {
                output.push_back(table[n & 0x3F]);
            }
        }

        return output;
    }

    std::string base64UrlDecode(const std::string& input) {
        static const int table[128] = {
            -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
            -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
            -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,62,-1,-1,
            52,53,54,55,56,57,58,59,60,61,-1,-1,-1,-1,-1,-1,
            -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,
            15,16,17,18,19,20,21,22,23,24,25,-1,-1,-1,-1,63,
            -1,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,
            41,42,43,44,45,46,47,48,49,50,51,-1,-1,-1,-1,-1
        };

        std::string output;
        output.reserve((input.size() * 3) / 4);

        uint32_t n = 0;
        int bits = 0;

        for (char c : input) {
            unsigned char uc = static_cast<unsigned char>(c);
            if (uc >= 128 || table[uc] == -1) {
                continue;
            }

            n = (n << 6) | table[uc];
            bits += 6;

            if (bits >= 8) {
                bits -= 8;
                output.push_back(static_cast<char>((n >> bits) & 0xFF));
            }
        }

        return output;
    }
};

} // namespace adapters::secondary
