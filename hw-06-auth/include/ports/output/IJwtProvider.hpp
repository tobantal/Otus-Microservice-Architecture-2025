#pragma once

#include <string>
#include <optional>

namespace ports::output {

/**
 * @brief Данные из JWT токена
 */
struct TokenClaims {
    std::string tokenId;
    std::string userId;
    std::string username;
    int64_t issuedAt;
    int64_t expiresAt;
};

/**
 * @brief Интерфейс провайдера JWT токенов
 * 
 * Упрощённая версия: только один тип токена.
 */
class IJwtProvider {
public:
    virtual ~IJwtProvider() = default;

    /**
     * @brief Создать токен
     */
    virtual std::string createToken(
        const std::string& userId,
        const std::string& username
    ) = 0;

    /**
     * @brief Проверить валидность токена
     */
    virtual bool validateToken(const std::string& token) = 0;

    /**
     * @brief Извлечь claims из токена
     */
    virtual std::optional<TokenClaims> extractClaims(const std::string& token) = 0;

    /**
     * @brief Добавить токен в blacklist (для logout)
     */
    virtual void blacklistToken(const std::string& token) = 0;

    /**
     * @brief Проверить, в blacklist ли токен
     */
    virtual bool isBlacklisted(const std::string& token) = 0;

    /**
     * @brief Получить время жизни токена в секундах
     */
    virtual int getTokenLifetime() const = 0;
};

} // namespace ports::output
