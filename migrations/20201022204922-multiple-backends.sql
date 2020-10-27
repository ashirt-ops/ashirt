-- +migrate Up

CREATE TABLE servers (
  uuid TEXT PRIMARY KEY NOT NULL,
  server_name TEXT NOT NULL,
  access_key TEXT NOT NULL,
  secret_key TEXT NOT NULL,
  host_path TEXT NOT NULL,
  deleted_at TIMESTAMP
)
;

-- these value MUST match the value specified by legacyServerUuid() and defaultServerName in constants.h
-- Also note that this value cannot be a variable within this file, due to sqlite limitation
INSERT INTO servers (uuid, server_name, access_key, secret_key, host_path) VALUES (
  '20a28c7c-ea24-4ee0-bb94-0ee63018d34b', 'default',
  '', '', '')
;

ALTER TABLE evidence ADD COLUMN server_uuid TEXT
;

UPDATE evidence SET server_uuid = '20a28c7c-ea24-4ee0-bb94-0ee63018d34b'
;


-- +migrate Down
DROP TABLE servers;
