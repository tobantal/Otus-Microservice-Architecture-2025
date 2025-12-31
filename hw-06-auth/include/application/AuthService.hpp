#pragma once

#include "ports/input/IAuthService.hpp"
#include "ports/output/IJwtProvider.hpp"
#include "ports/output/IUserRepository.hpp"
#include "ports/output/IProfileRepository.hpp"
#include <memory>
#include <iostream>
#include <random>
#include <sstream>
#include <iomanip>
#include <regex>
#include <functional>

namespace application {

/**
 * @brief Сервис аутентификации
 * 
 * Упрощённая версия для ДЗ-06:
 * - Регистрация создаёт User + пустой Profile
 * - Login возвращает один токен
 * - Токен содержит userId для доступа к профилю
 */
class AuthService : public ports::input::IAuthService {
public:
    AuthService(
        std::shared_ptr<ports::output::IJwtProvider> jwtProvider,
        std::shared_ptr<ports::output::IUserRepository> userRepository,
        std::shared_ptr<ports::output::IProfileRepository> profileRepository
    ) : jwtProvider_(std::move(jwtProvider))
      , userRepository_(std::move(userRepository))
      , profileRepository_(std::move(profileRepository))
      , rng_(std::random_device{}())
    {
        std::cout << "[AuthService] Created" << std::endl;
    }

    // ========================================================================
    // РЕГИСТРАЦИЯ
    // ========================================================================

    RegisterResult registerUser(
        const std::string& username,
        const std::string& password
    ) override {
        RegisterResult result;

        // Валидация username
        if (!isValidUsername(username)) {
            result.success = false;
            result.error = "Invalid username. Use 3-50 characters: a-z, A-Z, 0-9, _";
            return result;
        }

        // Проверка уникальности
        if (userRepository_->existsByUsername(username)) {
            result.success = false;
            result.error = "Username already exists";
            return result;
        }

        // Валидация пароля
        if (password.length() < 6) {
            result.success = false;
            result.error = "Password must be at least 6 characters";
            return result;
        }

        // Создание пользователя
        std::string userId = generateUuid();
        std::string passwordHash = hashPassword(password);

        domain::User user(userId, username, passwordHash);
        userRepository_->save(user);

        // Создание пустого профиля
        profileRepository_->createEmpty(userId);

        result.success = true;
        result.userId = userId;

        std::cout << "[AuthService] User registered: " << username 
                  << " (id=" << userId << ")" << std::endl;

        return result;
    }

    // ========================================================================
    // ВХОД
    // ========================================================================

    LoginResult login(
        const std::string& username,
        const std::string& password
    ) override {
        LoginResult result;

        // Поиск пользователя
        auto user = userRepository_->findByUsername(username);
        if (!user) {
            result.success = false;
            result.error = "Invalid username or password";
            return result;
        }

        // Проверка пароля
        if (!verifyPassword(password, user->passwordHash)) {
            result.success = false;
            result.error = "Invalid username or password";
            return result;
        }

        // Создание токена
        std::string token = jwtProvider_->createToken(user->id, user->username);

        result.success = true;
        result.token = token;
        result.tokenType = "Bearer";
        result.expiresIn = jwtProvider_->getTokenLifetime();
        result.userId = user->id;

        std::cout << "[AuthService] User logged in: " << username << std::endl;

        return result;
    }

    // ========================================================================
    // ВЫХОД
    // ========================================================================

    bool logout(const std::string& token) override {
        if (!jwtProvider_->validateToken(token)) {
            return false;
        }

        jwtProvider_->blacklistToken(token);
        std::cout << "[AuthService] User logged out" << std::endl;
        return true;
    }

    // ========================================================================
    // ВАЛИДАЦИЯ ТОКЕНА
    // ========================================================================

    ValidateResult validateToken(const std::string& token) override {
        ValidateResult result;

        if (jwtProvider_->isBlacklisted(token)) {
            result.valid = false;
            result.error = "Token has been revoked";
            return result;
        }

        if (!jwtProvider_->validateToken(token)) {
            result.valid = false;
            result.error = "Invalid or expired token";
            return result;
        }

        auto claims = jwtProvider_->extractClaims(token);
        if (!claims) {
            result.valid = false;
            result.error = "Cannot extract token claims";
            return result;
        }

        result.valid = true;
        result.userId = claims->userId;
        result.username = claims->username;

        return result;
    }

    // ========================================================================
    // ПОЛУЧЕНИЕ userId ИЗ ТОКЕНА
    // ========================================================================

    std::optional<std::string> getUserIdFromToken(const std::string& token) override {
        auto result = validateToken(token);
        if (!result.valid) {
            return std::nullopt;
        }
        return result.userId;
    }

private:
    std::shared_ptr<ports::output::IJwtProvider> jwtProvider_;
    std::shared_ptr<ports::output::IUserRepository> userRepository_;
    std::shared_ptr<ports::output::IProfileRepository> profileRepository_;
    std::mt19937_64 rng_;

    /**
     * @brief Валидация username
     */
    bool isValidUsername(const std::string& username) const {
        if (username.length() < 3 || username.length() > 50) {
            return false;
        }
        static const std::regex pattern("^[a-zA-Z0-9_]+$");
        return std::regex_match(username, pattern);
    }

    /**
     * @brief Генерация UUID
     */
    std::string generateUuid() {
        std::uniform_int_distribution<uint64_t> dist;
        std::stringstream ss;
        ss << std::hex << std::setfill('0');
        ss << std::setw(8) << (dist(rng_) & 0xFFFFFFFF) << "-";
        ss << std::setw(4) << (dist(rng_) & 0xFFFF) << "-";
        ss << std::setw(4) << ((dist(rng_) & 0x0FFF) | 0x4000) << "-";
        ss << std::setw(4) << ((dist(rng_) & 0x3FFF) | 0x8000) << "-";
        ss << std::setw(12) << (dist(rng_) & 0xFFFFFFFFFFFF);
        return ss.str();
    }

    /**
     * @brief Хеширование пароля (простой SHA256-like для демо)
     */
    std::string hashPassword(const std::string& password) const {
        // Простая реализация для демо — в production использовать bcrypt
        std::hash<std::string> hasher;
        size_t hash = hasher(password + "hw06_salt");
        std::stringstream ss;
        ss << std::hex << hash;
        return ss.str();
    }

    /**
     * @brief Проверка пароля
     */
    bool verifyPassword(const std::string& password, const std::string& hash) const {
        return hashPassword(password) == hash;
    }
};

} // namespace application
