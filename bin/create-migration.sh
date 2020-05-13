#! /usr/bin/env bash

# exit on error
set -e

# set cwd to project root
cd "$(dirname "$0")/.."

now=$(date -u +%Y%m%d%H%M%S)

desc="$*"

if [ -z "$desc" ]; then
	read -p 'Migration name: ' desc
fi

# sanitize description (spaces -> dashes)
desc=${desc// /-}

migrationsPath="./migrations"

filename="$now-$desc.sql"
filepath="$migrationsPath/$filename"


touch $filepath
echo "-- +migrate Up" >> $filepath
echo "" >> $filepath
echo "-- +migrate Down" >> $filepath

resourceFile="$(pwd)/res_migrations.qrc"

./bin/update_migration_resource.py "$resourceFile" "migrations/$filename"
