#pragma once

#include <string>
#include <optional>

namespace ports::input {

/**
 * @brief Интерфейс сервиса аутентификации
 * 
 * Упрощённая версия для ДЗ-06:
 * - Регистрация пользователя
 * - Вход (получение токена)
 * - Выход (инвалидация токена)
 * - Валидация токена
 */
class IAuthService {
public:
    virtual ~IAuthService() = default;

    // ========================================================================
    // РЕЗУЛЬТАТЫ ОПЕРАЦИЙ
    // ========================================================================

    struct RegisterResult {
        bool success = false;
        std::string error;
        std::string userId;
    };

    struct LoginResult {
        bool success = false;
        std::string error;
        std::string token;
        std::string tokenType = "Bearer";
        int expiresIn = 0;
        std::string userId;
    };

    struct ValidateResult {
        bool valid = false;
        std::string error;
        std::string userId;
        std::string username;
    };

    // ========================================================================
    // ОПЕРАЦИИ
    // ========================================================================

    /**
     * @brief Зарегистрировать нового пользователя
     */
    virtual RegisterResult registerUser(
        const std::string& username,
        const std::string& password
    ) = 0;

    /**
     * @brief Войти в систему
     */
    virtual LoginResult login(
        const std::string& username,
        const std::string& password
    ) = 0;

    /**
     * @brief Выйти из системы (инвалидировать токен)
     */
    virtual bool logout(const std::string& token) = 0;

    /**
     * @brief Проверить валидность токена
     */
    virtual ValidateResult validateToken(const std::string& token) = 0;

    /**
     * @brief Получить userId из токена
     */
    virtual std::optional<std::string> getUserIdFromToken(const std::string& token) = 0;
};

} // namespace ports::input
