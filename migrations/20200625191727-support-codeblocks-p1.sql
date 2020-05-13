-- +migrate Up
ALTER TABLE screenshots RENAME TO evidence;

-- +migrate Down
ALTER TABLE evidence RENAME TO screenshots;
