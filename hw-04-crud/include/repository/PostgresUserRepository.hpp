#pragma once

#include "repository/IUserRepository.hpp"
#include <pqxx/pqxx>
#include <memory>
#include <mutex>

namespace repository {

/**
 * @brief PostgreSQL реализация репозитория пользователей
 * 
 * Thread-safe благодаря мьютексу на соединение.
 * 
 * Требования:
 * - PostgreSQL 12+
 * - libpqxx
 * - Таблица users (см. sql/init.sql)
 */
class PostgresUserRepository : public IUserRepository {
public:
    /**
     * @brief Конструктор с параметрами подключения
     * @param host Хост PostgreSQL
     * @param port Порт PostgreSQL
     * @param dbname Имя базы данных
     * @param user Пользователь
     * @param password Пароль
     */
    PostgresUserRepository(
        const std::string& host,
        int port,
        const std::string& dbname,
        const std::string& user,
        const std::string& password
    );

    /**
     * @brief Конструктор с connection string
     * @param connectionString Строка подключения PostgreSQL
     */
    explicit PostgresUserRepository(const std::string& connectionString);

    ~PostgresUserRepository() override;

    // IUserRepository implementation
    int64_t create(const domain::User& user) override;
    std::optional<domain::User> findById(int64_t id) override;
    std::vector<domain::User> findAll() override;
    bool update(int64_t id, const domain::User& user) override;
    bool deleteById(int64_t id) override;

    /**
     * @brief Проверить соединение с БД
     */
    bool isConnected() const;

private:
    /**
     * @brief Преобразовать строку результата в User
     */
    domain::User rowToUser(const pqxx::row& row) const;

    std::unique_ptr<pqxx::connection> connection_;
    mutable std::mutex mutex_;
};

} // namespace repository
