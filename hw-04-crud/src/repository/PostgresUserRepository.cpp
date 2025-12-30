#include "repository/PostgresUserRepository.hpp"
#include <iostream>
#include <sstream>

namespace repository {

PostgresUserRepository::PostgresUserRepository(
    const std::string& host,
    int port,
    const std::string& dbname,
    const std::string& user,
    const std::string& password
) {
    std::ostringstream connStr;
    connStr << "host=" << host
            << " port=" << port
            << " dbname=" << dbname
            << " user=" << user
            << " password=" << password;
    
    std::cout << "[PostgresUserRepo] Connecting to PostgreSQL at " 
              << host << ":" << port << "/" << dbname << std::endl;
    
    try {
        connection_ = std::make_unique<pqxx::connection>(connStr.str());
        std::cout << "[PostgresUserRepo] Connected successfully" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "[PostgresUserRepo] Connection failed: " << e.what() << std::endl;
        throw;
    }
}

PostgresUserRepository::PostgresUserRepository(const std::string& connectionString) {
    std::cout << "[PostgresUserRepo] Connecting to PostgreSQL..." << std::endl;
    
    try {
        connection_ = std::make_unique<pqxx::connection>(connectionString);
        std::cout << "[PostgresUserRepo] Connected successfully" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "[PostgresUserRepo] Connection failed: " << e.what() << std::endl;
        throw;
    }
}

PostgresUserRepository::~PostgresUserRepository() {
    // Соединение закрывается автоматически деструктором pqxx::connection
    std::cout << "[PostgresUserRepo] Connection closed" << std::endl;
}

int64_t PostgresUserRepository::create(const domain::User& user) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    try {
        pqxx::work txn(*connection_);
        
        auto result = txn.exec_params(
            R"(
                INSERT INTO users (username, first_name, last_name, email, phone)
                VALUES ($1, $2, $3, $4, $5)
                RETURNING id
            )",
            user.username,
            user.firstName,
            user.lastName,
            user.email,
            user.phone
        );
        
        txn.commit();
        
        int64_t id = result[0][0].as<int64_t>();
        std::cout << "[PostgresUserRepo] Created user with id=" << id << std::endl;
        return id;
        
    } catch (const std::exception& e) {
        std::cerr << "[PostgresUserRepo] create() failed: " << e.what() << std::endl;
        throw;
    }
}

std::optional<domain::User> PostgresUserRepository::findById(int64_t id) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    try {
        pqxx::work txn(*connection_);
        
        auto result = txn.exec_params(
            R"(
                SELECT id, username, first_name, last_name, email, phone
                FROM users
                WHERE id = $1
            )",
            id
        );
        
        txn.commit();
        
        if (result.empty()) {
            return std::nullopt;
        }
        
        return rowToUser(result[0]);
        
    } catch (const std::exception& e) {
        std::cerr << "[PostgresUserRepo] findById() failed: " << e.what() << std::endl;
        return std::nullopt;
    }
}

std::vector<domain::User> PostgresUserRepository::findAll() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::vector<domain::User> users;
    
    try {
        pqxx::work txn(*connection_);
        
        auto result = txn.exec(
            R"(
                SELECT id, username, first_name, last_name, email, phone
                FROM users
                ORDER BY id ASC
            )"
        );
        
        txn.commit();
        
        for (const auto& row : result) {
            users.push_back(rowToUser(row));
        }
        
    } catch (const std::exception& e) {
        std::cerr << "[PostgresUserRepo] findAll() failed: " << e.what() << std::endl;
    }
    
    return users;
}

bool PostgresUserRepository::update(int64_t id, const domain::User& user) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    try {
        pqxx::work txn(*connection_);
        
        auto result = txn.exec_params(
            R"(
                UPDATE users
                SET username = $2,
                    first_name = $3,
                    last_name = $4,
                    email = $5,
                    phone = $6,
                    updated_at = NOW()
                WHERE id = $1
            )",
            id,
            user.username,
            user.firstName,
            user.lastName,
            user.email,
            user.phone
        );
        
        txn.commit();
        
        bool updated = result.affected_rows() > 0;
        if (updated) {
            std::cout << "[PostgresUserRepo] Updated user id=" << id << std::endl;
        }
        return updated;
        
    } catch (const std::exception& e) {
        std::cerr << "[PostgresUserRepo] update() failed: " << e.what() << std::endl;
        return false;
    }
}

bool PostgresUserRepository::deleteById(int64_t id) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    try {
        pqxx::work txn(*connection_);
        
        auto result = txn.exec_params(
            "DELETE FROM users WHERE id = $1",
            id
        );
        
        txn.commit();
        
        bool deleted = result.affected_rows() > 0;
        if (deleted) {
            std::cout << "[PostgresUserRepo] Deleted user id=" << id << std::endl;
        }
        return deleted;
        
    } catch (const std::exception& e) {
        std::cerr << "[PostgresUserRepo] deleteById() failed: " << e.what() << std::endl;
        return false;
    }
}

bool PostgresUserRepository::isConnected() const {
    return connection_ && connection_->is_open();
}

domain::User PostgresUserRepository::rowToUser(const pqxx::row& row) const {
    domain::User user;
    user.id = row["id"].as<int64_t>();
    user.username = row["username"].as<std::string>();
    user.firstName = row["first_name"].is_null() ? "" : row["first_name"].as<std::string>();
    user.lastName = row["last_name"].is_null() ? "" : row["last_name"].as<std::string>();
    user.email = row["email"].as<std::string>();
    user.phone = row["phone"].is_null() ? "" : row["phone"].as<std::string>();
    return user;
}

} // namespace repository
