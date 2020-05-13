-- +migrate Up
CREATE TABLE screenshots (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    path TEXT NOT NULL,
    operation_slug TEXT NOT NULL,
    description TEXT NOT NULL DEFAULT '',
    error TEXT NOT NULL DEFAULT '',
    recorded_date TIMESTAMP NOT NULL,
    upload_date TIMESTAMP
);

-- +migrate Down
DROP TABLE screenshots;
