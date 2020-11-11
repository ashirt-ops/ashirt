// Copyright 2020, Verizon Media
// Licensed under the terms of MIT. See LICENSE file in project root for terms.

#include "databaseconnection.h"

#include <QDir>
#include <QVariant>
#include <iostream>
#include <unordered_map>
#include <vector>

#include "exceptions/databaseerr.h"
#include "exceptions/fileerror.h"
#include "helpers/file_helpers.h"

// DatabaseConnection constructs a connection to the database, unsurpringly. Note that the
// constructor can throw a error (see below). Additionally, many methods can throw a QSqlError,
// though are not marked as such in their comments. Other errors are listed in throw comments, or
// are marked as noexcept if no error is possible.
//
// Throws: DBDriverUnavailable if the required database driver does not exist
DatabaseConnection::DatabaseConnection(QString dbPath, QString databaseName) {
  const static QString dbDriver = "QSQLITE";
  if (QSqlDatabase::isDriverAvailable(dbDriver)) {
    dbName = databaseName;
    auto db = QSqlDatabase::addDatabase(dbDriver, dbName);
    QDir().mkpath(FileHelpers::getDirname(dbPath));
    db.setDatabaseName(dbPath);
    this->_dbPath = dbPath;
  }
  else {
    throw DBDriverUnavailableError("SQLite");
  }
}

void DatabaseConnection::withConnection(QString dbPath, QString dbName, std::function<void(DatabaseConnection)>actions) {
  DatabaseConnection conn(dbPath, dbName);
  conn.connect();
  try {
    actions(conn);
  }
  catch(const std::runtime_error& e) {
    std::cerr << "Ran into an error dealing with database actions: " << e.what() << std::endl;
  }

  conn.close();
  QSqlDatabase::removeDatabase(dbName);
}

void DatabaseConnection::connect() {
  auto db = getDB();
  if (!db.open()) {
    throw db.lastError();
  }
  migrateDB();
}

void DatabaseConnection::close() noexcept { getDB().close(); }

qint64 DatabaseConnection::createEvidence(const QString &filepath, const QString &operationSlug,
                                          const QString &contentType) {
  return doInsert(getDB(),
                  "INSERT INTO evidence"
                  " (path, operation_slug, content_type, recorded_date)"
                  " VALUES"
                  " (?, ?, ?, datetime('now'))",
                  {filepath, operationSlug, contentType});
}

qint64 DatabaseConnection::createFullEvidence(const model::Evidence &evidence) {
  return doInsert(getDB(),
                  "INSERT INTO evidence"
                  " (path, operation_slug, content_type, description, error, recorded_date, upload_date)"
                  " VALUES"
                  " (?, ?, ?, ?, ?, ?, ?)",
                  {evidence.path, evidence.operationSlug, evidence.contentType, evidence.description,
                   evidence.errorText, evidence.recordedDate, evidence.uploadDate});
}

qint64 DatabaseConnection::createFullEvidenceWithID(const model::Evidence &evidence) {
  return doInsert(getDB(),
                  "INSERT INTO evidence"
                  " (id, path, operation_slug, content_type, description, error, recorded_date, upload_date)"
                  " VALUES"
                  " (?, ?, ?, ?, ?, ?, ?)",
                  {evidence.id, evidence.path, evidence.operationSlug, evidence.contentType,
                   evidence.description, evidence.errorText, evidence.recordedDate,
                   evidence.uploadDate});
}


model::Evidence DatabaseConnection::getEvidenceDetails(qint64 evidenceID) {
  model::Evidence rtn;
  auto query = executeQuery(getDB(),
      "SELECT"
      " id, path, operation_slug, content_type, description, error, recorded_date, upload_date"
      " FROM evidence"
      " WHERE id=? LIMIT 1",
      {evidenceID});

  if (query.first()) {
    rtn.id = query.value("id").toLongLong();
    rtn.path = query.value("path").toString();
    rtn.operationSlug = query.value("operation_slug").toString();
    rtn.contentType = query.value("content_type").toString();
    rtn.description = query.value("description").toString();
    rtn.errorText = query.value("error").toString();
    rtn.recordedDate = query.value("recorded_date").toDateTime();
    rtn.uploadDate = query.value("upload_date").toDateTime();

    rtn.recordedDate.setTimeSpec(Qt::UTC);
    rtn.uploadDate.setTimeSpec(Qt::UTC);

    rtn.tags = getTagsForEvidenceID(evidenceID);
  }
  else {
    std::cerr << "Could not find evidence with id: " << evidenceID << std::endl;
  }
  return rtn;
}

void DatabaseConnection::updateEvidenceDescription(const QString &newDescription,
                                                   qint64 evidenceID) {
  executeQuery(getDB(), "UPDATE evidence SET description=? WHERE id=?", {newDescription, evidenceID});
}

void DatabaseConnection::deleteEvidence(qint64 evidenceID) {
  executeQuery(getDB(), "DELETE FROM evidence WHERE id=?", {evidenceID});
}

void DatabaseConnection::updateEvidenceError(const QString &errorText, qint64 evidenceID) {
  executeQuery(getDB(), "UPDATE evidence SET error=? WHERE id=?", {errorText, evidenceID});
}

void DatabaseConnection::updateEvidenceSubmitted(qint64 evidenceID) {
  executeQuery(getDB(), "UPDATE evidence SET upload_date=datetime('now') WHERE id=?", {evidenceID});
}

std::vector<model::Tag> DatabaseConnection::getTagsForEvidenceID(qint64 evidenceID) {
  std::vector<model::Tag> tags;
  auto getTagQuery = executeQuery(getDB(), "SELECT id, tag_id, name FROM tags WHERE evidence_id=?",
                                  {evidenceID});
  while (getTagQuery.next()) {
    auto tag = model::Tag(getTagQuery.value("id").toLongLong(),
                          getTagQuery.value("tag_id").toLongLong(),
                          getTagQuery.value("name").toString());
    tags.emplace_back(tag);
  }
  return tags;
}

void DatabaseConnection::setEvidenceTags(const std::vector<model::Tag> &newTags,
                                         qint64 evidenceID) {
  auto db = getDB();
  QList<QVariant> newTagIds;
  for (const auto &tag : newTags) {
    newTagIds.push_back(tag.serverTagId);
  }
  executeQuery(db, "DELETE FROM tags WHERE tag_id NOT IN (?) AND evidence_id = ?",
               {newTagIds, evidenceID});

  auto currentTagsResult =
      executeQuery(db, "SELECT tag_id FROM tags WHERE evidence_id = ?", {evidenceID});
  QList<qint64> currentTags;
  while (currentTagsResult.next()) {
    currentTags.push_back(currentTagsResult.value("tag_id").toLongLong());
  }
  struct dataset {
    qint64 evidenceID = 0;
    qint64 tagID = 0;
    QString name;
  };
  std::vector<dataset> tagDataToInsert;
  QString baseQuery = "INSERT INTO tags (evidence_id, tag_id, name) VALUES ";
  for (const auto &newTag : newTags) {
    if (currentTags.count(newTag.serverTagId) == 0) {
      dataset item;
      item.evidenceID = evidenceID;
      item.tagID = newTag.serverTagId;
      item.name = newTag.tagName;
      tagDataToInsert.push_back(item);
    }
  }

  // one possible concern: we are going to be passing a lot of parameters
  // sqlite indicates it's default is 100 passed parameter, but it can "handle thousands"
  if (!tagDataToInsert.empty()) {
    std::vector<QVariant> args;
    baseQuery += "(?,?,?)";
    baseQuery += QString(", (?,?,?)").repeated(int(tagDataToInsert.size() - 1));
    for (const auto &item : tagDataToInsert) {
      args.emplace_back(item.evidenceID);
      args.emplace_back(item.tagID);
      args.emplace_back(item.name);
    }
    executeQuery(db, baseQuery, args);
  }
}

DBQuery DatabaseConnection::buildGetEvidenceWithFiltersQuery(const EvidenceFilters &filters) {
  QString query =
      "SELECT"
      " id, path, operation_slug, content_type, description, error, recorded_date, upload_date"
      " FROM evidence";
  std::vector<QVariant> values;
  std::vector<QString> parts;

  if (filters.hasError != Tri::Any) {
    parts.emplace_back(" error LIKE ? ");
    // _% will ensure at least one character exists in the error column, ensuring it's populated
    values.emplace_back(filters.hasError == Tri::Yes ? "_%" : "");
  }
  if (filters.submitted != Tri::Any) {
    parts.emplace_back((filters.submitted == Tri::Yes) ? " upload_date IS NOT NULL "
                                                       : " upload_date IS NULL ");
  }
  if (!filters.operationSlug.isEmpty()) {
    parts.emplace_back(" operation_slug = ? ");
    values.emplace_back(filters.operationSlug);
  }
  if (!filters.contentType.isEmpty()) {
    parts.emplace_back(" content_type = ? ");
    values.emplace_back(filters.contentType);
  }
  if (filters.startDate.isValid()) {
    parts.emplace_back(" recorded_date >= ? ");
    values.emplace_back(filters.startDate);
  }
  if (filters.endDate.isValid()) {
    auto realEndDate = filters.endDate.addDays(1);
    parts.emplace_back(" recorded_date < ? ");
    values.emplace_back(realEndDate);
  }

  if (!parts.empty()) {
    query += " WHERE " + parts.at(0);
    for (size_t i = 1; i < parts.size(); i++) {
      query += " AND " + parts.at(i);
    }
  }
  return DBQuery(query, values);
}

void DatabaseConnection::updateEvidencePath(QString newPath, qint64 evidenceID) {
  executeQuery(getDB(), "UPDATE evidence SET path=? WHERE id=?", {newPath, evidenceID});
}

std::vector<model::Evidence> DatabaseConnection::getEvidenceWithFilters(
    const EvidenceFilters &filters) {
  auto dbQuery = buildGetEvidenceWithFiltersQuery(filters);
  auto resultSet = executeQuery(getDB(), dbQuery.query(), dbQuery.values());

  std::vector<model::Evidence> allEvidence;
  while (resultSet.next()) {
    model::Evidence evi;
    evi.id = resultSet.value("id").toLongLong();
    evi.path = resultSet.value("path").toString();
    evi.operationSlug = resultSet.value("operation_slug").toString();
    evi.contentType = resultSet.value("content_type").toString();
    evi.description = resultSet.value("description").toString();
    evi.errorText = resultSet.value("error").toString();
    evi.recordedDate = resultSet.value("recorded_date").toDateTime();
    evi.uploadDate = resultSet.value("upload_date").toDateTime();

    evi.recordedDate.setTimeSpec(Qt::UTC);
    evi.uploadDate.setTimeSpec(Qt::UTC);

    allEvidence.push_back(evi);
  }

  return allEvidence;
}

std::vector<model::Evidence> DatabaseConnection::createEvidenceExportView(QString pathToExport, EvidenceFilters filters, DatabaseConnection* runningDB) {
  std::vector<model::Evidence> exportEvidence;
  std::unordered_map<qint64, qint64> oldIDToNewID;

  auto exportViewAction = [runningDB, filters, &exportEvidence, &oldIDToNewID](DatabaseConnection exportDB) {
    exportEvidence = runningDB->getEvidenceWithFilters(filters);

    // TODO: The below is an "N+1" problem. We should instead do multi-row insert(s). However, the
    // evidence id mapping poses a problem. We may *not* need this mapping. The tag insert below
    // is *also* an n+1 problem that should be resolved, but how this happens is less clear.
    for (auto& evi : exportEvidence) {
      int newId = exportDB.createFullEvidence(evi);
      oldIDToNewID.emplace(evi.id, newId);
      evi.id = newId;
    }
    for(auto eviIDsPair : oldIDToNewID) {
      auto oldId = eviIDsPair.first;
      auto newId = eviIDsPair.second;

      auto tags = runningDB->getTagsForEvidenceID(oldId);
      exportDB.setEvidenceTags(tags, newId);
    }
  };

  withConnection(pathToExport, "exportDB", exportViewAction);

  return exportEvidence;
}

// migrateDB checks the migration status and then performs the full migration for any
// lacking update.
//
// Throws exceptions/FileError if a migration file cannot be found.
void DatabaseConnection::migrateDB() {
  auto db = getDB();
  std::cout << "Checking database state" << std::endl;
  auto migrationsToApply = DatabaseConnection::getUnappliedMigrations(db);

  for (const QString &newMigration : migrationsToApply) {
    QFile migrationFile(":/migrations/" + newMigration);
    auto ok = migrationFile.open(QFile::ReadOnly);
    if (!ok) {
      throw FileError::mkError("Error reading migration file",
                               migrationFile.fileName().toStdString(), migrationFile.error());
    }
    auto content = QString(migrationFile.readAll());
    migrationFile.close();

    std::cout << "Applying Migration: " << newMigration.toStdString() << std::endl;
    auto upScript = extractMigrateUpContent(content);
    executeQuery(db, upScript);
    executeQuery(db,
                 "INSERT INTO migrations (migration_name, applied_at) VALUES (?, datetime('now'))",
                 {newMigration});
  }
  std::cout << "All migrations applied" << std::endl;
}

// getUnappliedMigrations retrieves a list of all of the migrations that have not been applied
// to the local database.
//
// Note: All sql files must end in ".sql" to be picked up
//
// Throws:
//   * BadDatabaseStateError if some migrations have been applied that are not known
//   * QSqlError if database queries fail
QStringList DatabaseConnection::getUnappliedMigrations(const QSqlDatabase &db) {
  QDir migrationsDir(":/migrations");

  auto allMigrations = migrationsDir.entryList(QDir::Files, QDir::Name);
  QStringList appliedMigrations;
  QStringList migrationsToApply;

  auto queryResult = executeQueryNoThrow(db, "SELECT migration_name FROM migrations");
  QSqlQuery* dbMigrations = &queryResult.query;
  while (queryResult.success && queryResult.query.next()) {
    appliedMigrations << dbMigrations->value("migration_name").toString();
  }
  // compare the two list to find gaps
  for (const QString &possibleMigration : allMigrations) {
    if (possibleMigration.right(4) != ".sql") {
      continue;  // assume non-sql files aren't actual migrations.
    }
    auto foundIndex = appliedMigrations.indexOf(possibleMigration);
    if (foundIndex == -1) {
      migrationsToApply << possibleMigration;
    }
    else {
      appliedMigrations.removeAt(foundIndex);
    }
  }
  if (!appliedMigrations.empty()) {
    throw BadDatabaseStateError();
  }
  return migrationsToApply;
}

// extractMigrateUpContent parses the given migration content and retrieves only
// the portion that applies to the "up" / apply logic. The "down" section is ignored.
QString DatabaseConnection::extractMigrateUpContent(const QString &allContent) noexcept {
  auto copying = false;
  QString upContent;
  for (const QString &line : allContent.split("\n")) {
    if (line.trimmed().toLower() == "-- +migrate up") {
      copying = true;
    }
    else if (line.trimmed().toLower() == "-- +migrate down") {
      if (copying) {
        break;
      }
      copying = false;
    }
    else if (copying) {
      upContent.append(line + "\n");
    }
  }
  return upContent;
}

// executeQuery simply attempts to execute the given stmt with the passed args. The statement is
// first prepared, and arg placements can be specified with "?"
//
// Throws: QSqlError when a query error occurs
QSqlQuery DatabaseConnection::executeQuery(const QSqlDatabase& db, const QString &stmt,
                                           const std::vector<QVariant> &args) {
  auto result = executeQueryNoThrow(db, stmt, args);
  if (!result.success) {
    throw result.err;
  }
  return result.query;
}

QueryResult DatabaseConnection::executeQueryNoThrow(const QSqlDatabase& db, const QString &stmt,
                                                  const std::vector<QVariant> &args) noexcept {
  QSqlQuery query(db);

  bool prepared = query.prepare(stmt);
  if (!prepared) {
    return QueryResult(std::move(query));
  }

  for (const auto &arg : args) {
    query.addBindValue(arg);
  }

  query.exec();
  return QueryResult(std::move(query));
}

// doInsert is a version of executeQuery that returns the last inserted id, rather than the
// underlying query/response
//
// Throws: QSqlError when a query error occurs
qint64 DatabaseConnection::doInsert(const QSqlDatabase& db, const QString &stmt,
                                    const std::vector<QVariant> &args) {
  auto query = executeQuery(db, stmt, args);

  return query.lastInsertId().toLongLong();
}

QSqlDatabase DatabaseConnection::getDB() {
  if (dbName == "") {
    return QSqlDatabase::database();
  }
  return QSqlDatabase::database(dbName);
}

QString DatabaseConnection::getDatabasePath() {
  return _dbPath;
}
