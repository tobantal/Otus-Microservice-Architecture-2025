-- HW06: Authentication Service Database Schema

-- Users table
CREATE TABLE IF NOT EXISTS users (
    id VARCHAR(36) PRIMARY KEY,
    username VARCHAR(50) UNIQUE NOT NULL,
    password_hash VARCHAR(255) NOT NULL,
    created_at TIMESTAMP DEFAULT NOW()
);

-- Profiles table
CREATE TABLE IF NOT EXISTS profiles (
    user_id VARCHAR(36) PRIMARY KEY REFERENCES users(id) ON DELETE CASCADE,
    first_name VARCHAR(100),
    last_name VARCHAR(100),
    email VARCHAR(255),
    phone VARCHAR(50),
    updated_at TIMESTAMP DEFAULT NOW()
);

-- Indexes
CREATE INDEX IF NOT EXISTS idx_users_username ON users(username);
CREATE INDEX IF NOT EXISTS idx_profiles_user_id ON profiles(user_id);

-- Optional: Test data (comment out for production)
-- INSERT INTO users (id, username, password_hash) VALUES 
--     ('test-user-001', 'testuser', 'testhash');
-- INSERT INTO profiles (user_id, first_name, last_name, email) VALUES 
--     ('test-user-001', 'Test', 'User', 'test@example.com');
