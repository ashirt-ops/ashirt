-- +migrate Up
CREATE TABLE tags (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    screenshot_id INTEGER NOT NULL,
    tag_id INTEGER NOT NULL,
    name TEXT NOT NULL
);

-- +migrate Down
DROP TABLE tags;
