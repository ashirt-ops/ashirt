-- +migrate Up
ALTER TABLE tags RENAME COLUMN screenshot_id TO evidence_id;

-- +migrate Down
ALTER TABLE tags RENAME COLUMN evidence_id TO screenshot_id;
