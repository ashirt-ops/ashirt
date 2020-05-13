# Developer Notes

## Build Requirements

This application is built off of Qt 5, and utilizes's Qt's networking and Sql featuresets. To build, your specific system may need the following:

1. Qt 5, `qmake`, and possibly Qt Creator IDE.
   1. Binaries located [here](https://www.qt.io/download-qt-installer?hsCtaTracking=99d9dd4f-5681-48d2-b096-470725510d34%7C074ddad0-fdef-4e53-8aa8-5e8a876d6ab4). You may need to alter which downloader you install. 
2. SQLite C driver (for SQLite version 3)
   1. On Fedora, this can be installed with `yum install sqlite-devel`
   2. On Arch systems, this can be installed with `pacman -S sqlite-doc`

## Adding a db migration

Updating the database schema is slightly complicated, and the following rules must be followed:

1. Use the helper script `bin/create-migration.sh`
   * This will create file in the `migrations` folder with the indicated name and a timestamp
   * This should also add this migration to the qrc file. However, if this is not done, you can do this manually by editing the `rs_migrations.qrc` file.
2. Inside the new migration file, add the necessary sql to apply the db change under `-- +migrate Up`
3. Inside the new migration file, add the necessary sql to _undo_ the db change under `-- +migrate Down`
4. Only one statement is allowed under each heading. If multiple statements need to be applied, they should done as multiple migration files
   * This is a sqlite3/Qt limitation.

## Adding a new Evidence Filter

Evidence filters require modification in a few files and functions. Here is a rough checklist:

|                          | File                   | Function               | Notes                                                               |
| ------------------------ | ---------------------- | ---------------------- | ------------------------------------------------------------------- |
| <input type="checkbox"/> | evidencefilter.h       |                        | Need to add `FILTER_KEY_` and `FILTER_KEYS_` values                 |
| <input type="checkbox"/> | evidencefilter.cpp     | standardizeFilterKey   | Needed to map filter key alias to the one true filter key           |
| <input type="checkbox"/> | evidencefilter.cpp     | toString               | Need to represent a filter key/value as a string                    |
| <input type="checkbox"/> | evidencefilter.cpp     | parseFilter            | Need to be able to read filter key/value from a string              |
| <input type="checkbox"/> | databaseconnection.cpp | getEvidenceWithFilters | Need to translate the filter key/value to an appropriate sql clause |

Currently, there is already built-in support for adding filters of type:

* String (use the Operation filter as a guide)
* Boolean/Tri (Tris represent Yes/No/Any here, use Error filter as a guide)
* Date Range (use To/From filters as a guide)

## Formatting

This application adopts a modified [Google code style](https://google.github.io/styleguide/cppguide.html), applied via `clang-format`. Note that while formatting style is adhered to, other parts may not be followed, due to not starting with this style in mind.

Google style changes:
* Line limit to 100 characters
  * This is simply easier to read when dealing with long lines
* Line breaks before `else` and `catch`.
  * This is mostly because I find these easier to read.

To apply code formatting (Linux/Bash), run `find src/ -iname "*.cpp" -o -iname "*.h" | xargs clang-format -i`

## Known Issues

1. Evidence manager columns aren't ideally sized
2. No CLI for non-qt/non-tray OSes
3. Remove dock icon on mac?
   1. Possibly useful: https://github.com/keepassxreboot/keepassxc/commit/45344bb2acea61806d5e7f5f9eadfa779ca536ae#diff-a9e708931297992b08350ff7122fcb91R157
