-- ============================================================================
-- init.sql - Инициализация схемы базы данных
-- ============================================================================
-- Выполняется как Kubernetes Job перед запуском приложения
-- ============================================================================

-- Создание таблицы пользователей
CREATE TABLE IF NOT EXISTS users (
    id          BIGSERIAL PRIMARY KEY,
    username    VARCHAR(255) NOT NULL UNIQUE,
    first_name  VARCHAR(255),
    last_name   VARCHAR(255),
    email       VARCHAR(255) NOT NULL,
    phone       VARCHAR(50),
    created_at  TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
    updated_at  TIMESTAMP WITH TIME ZONE DEFAULT NOW()
);

-- Индексы для ускорения поиска
CREATE INDEX IF NOT EXISTS idx_users_username ON users(username);
CREATE INDEX IF NOT EXISTS idx_users_email ON users(email);

-- Комментарии к таблице
COMMENT ON TABLE users IS 'Таблица пользователей для CRUD операций';
COMMENT ON COLUMN users.id IS 'Уникальный идентификатор пользователя';
COMMENT ON COLUMN users.username IS 'Логин пользователя (уникальный)';
COMMENT ON COLUMN users.first_name IS 'Имя';
COMMENT ON COLUMN users.last_name IS 'Фамилия';
COMMENT ON COLUMN users.email IS 'Email адрес';
COMMENT ON COLUMN users.phone IS 'Номер телефона';
