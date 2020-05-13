-- +migrate Up
UPDATE evidence SET content_type='image';
-- +migrate Down
