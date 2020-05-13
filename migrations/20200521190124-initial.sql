-- +migrate Up
CREATE TABLE migrations (
    migration_name TEXT NOT NULL,
    applied_at TIMESTAMP
);

-- +migrate Down
DROP TABLE migrations;
