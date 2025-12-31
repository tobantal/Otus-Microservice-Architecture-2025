-- HW05 Monitoring: Database initialization
-- Таблица users (идентично hw04)

CREATE TABLE IF NOT EXISTS users (
    id SERIAL PRIMARY KEY,
    username VARCHAR(255) UNIQUE NOT NULL,
    first_name VARCHAR(255),
    last_name VARCHAR(255),
    email VARCHAR(255) UNIQUE NOT NULL,
    phone VARCHAR(50),
    created_at TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
    updated_at TIMESTAMP WITH TIME ZONE DEFAULT NOW()
);

-- Индексы
CREATE INDEX IF NOT EXISTS idx_users_username ON users(username);
CREATE INDEX IF NOT EXISTS idx_users_email ON users(email);

-- Тестовые данные
INSERT INTO users (username, first_name, last_name, email, phone)
VALUES 
    ('johndoe', 'John', 'Doe', 'john.doe@example.com', '+1-555-0101'),
    ('janedoe', 'Jane', 'Doe', 'jane.doe@example.com', '+1-555-0102'),
    ('testuser', 'Test', 'User', 'test@example.com', '+1-555-0103')
ON CONFLICT (username) DO NOTHING;
