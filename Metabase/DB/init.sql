CREATE TABLE IF NOT EXISTS sensors (
    id SERIAL PRIMARY KEY,
    sensor_id VARCHAR(32),
    temperature REAL,
    pressure REAL,
    humidity REAL,
    sound REAL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);