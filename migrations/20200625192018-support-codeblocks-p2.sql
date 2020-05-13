-- +migrate Up
ALTER TABLE evidence ADD COLUMN content_type TEXT NOT NULL DEFAULT 'image';

-- +migrate Down
-- cannot do a proper migrate down (SQLite does not support ALTER TABLE DROP COLUMN)
