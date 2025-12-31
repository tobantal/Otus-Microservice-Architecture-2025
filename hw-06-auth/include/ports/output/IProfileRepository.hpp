#pragma once

#include "domain/Profile.hpp"
#include <optional>
#include <string>

namespace ports::output {

/**
 * @brief Интерфейс репозитория профилей
 */
class IProfileRepository {
public:
    virtual ~IProfileRepository() = default;

    /**
     * @brief Сохранить или обновить профиль
     */
    virtual void save(const domain::Profile& profile) = 0;

    /**
     * @brief Найти профиль по userId
     */
    virtual std::optional<domain::Profile> findByUserId(const std::string& userId) = 0;

    /**
     * @brief Создать пустой профиль для нового пользователя
     */
    virtual void createEmpty(const std::string& userId) = 0;
};

} // namespace ports::output
