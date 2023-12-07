#include "databaseconnection.h"

#include <QDir>
#include <QVariant>

#include "helpers/file_helpers.h"

DatabaseConnection::DatabaseConnection(const QString& dbPath, const QString& databaseName)
 : _dbName(databaseName)
 , _dbPath(dbPath)
 , _db(QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), databaseName))
{
    const auto dbDir = FileHelpers::getDirname(_dbPath);
    if(!QDir().exists(dbDir))
        QDir().mkpath(dbDir);
    _db.setDatabaseName(_dbPath);
}

bool DatabaseConnection::withConnection(const QString& dbPath, const QString &dbName,
                                        const std::function<void(DatabaseConnection)> &actions)
{
    DatabaseConnection conn(dbPath, dbName);
    if(!conn.connect())
        return false;
    actions(conn);
    bool rtn = true;
    if( conn._db.lastError().type() != QSqlError::NoError)
        rtn = false;

    conn.close();
    QSqlDatabase::removeDatabase(dbPath);
    return rtn;
}

bool DatabaseConnection::connect()
{
    if (!_db.open())
        return false;
    return migrateDB();
}

qint64 DatabaseConnection::createEvidence(const QString &filepath, const QString &operationSlug, const QString &contentType)
{
    auto qKeys = QStringLiteral("path, operation_slug, content_type, recorded_date");
    auto qValues = QStringLiteral("?, ?, ?, datetime('now')");
    auto qStr = _sqlBasicInsert.arg(_tblEvidence, qKeys, qValues);
    return doInsert(_db, qStr, {filepath, operationSlug, contentType});
}

qint64 DatabaseConnection::createFullEvidence(const model::Evidence &evidence) {
    auto qKeys = QStringLiteral("path, operation_slug, content_type, description, error, recorded_date, upload_date");
    auto qValues = QStringLiteral("?, ?, ?, ?, ?, ?, ?");
    auto qStr = _sqlBasicInsert.arg(_tblEvidence, qKeys, qValues);
    return doInsert(_db, qStr,
                  {evidence.path, evidence.operationSlug, evidence.contentType, evidence.description,
                   evidence.errorText, evidence.recordedDate, evidence.uploadDate});
}

void DatabaseConnection::batchCopyFullEvidence(const QList<model::Evidence> &evidence) {
  auto baseQuery = QStringLiteral("INSERT INTO evidence (%1) VALUES %2").arg(_evidenceAllKeys, QStringLiteral("%1"));
  int varsPerRow = 8; // count number of "?"
  std::function<QVariantList(int)> getItemValues = [evidence](int i){
    auto item = evidence.at(i);
    return QVariantList {
        item.id, item.path, item.operationSlug, item.contentType, item.description,
        item.errorText, item.recordedDate, item.uploadDate
    };
  };
  batchInsert(baseQuery, varsPerRow, evidence.size(), getItemValues);
}


model::Evidence DatabaseConnection::getEvidenceDetails(qint64 evidenceID)
{
  model::Evidence rtn;
  auto qStr = QStringLiteral("%1 WHERE id=? LIMIT 1").arg(_sqlSelectTemplate.arg(_evidenceAllKeys, _tblEvidence));
  auto query = executeQuery(_db, qStr, {evidenceID});
  if (_db.lastError().type() == QSqlError::NoError && query.first()) {
    rtn.id = query.value(QStringLiteral("id")).toLongLong();
    rtn.path = query.value(QStringLiteral("path")).toString();
    rtn.operationSlug = query.value(QStringLiteral("operation_slug")).toString();
    rtn.contentType = query.value(QStringLiteral("content_type")).toString();
    rtn.description = query.value(QStringLiteral("description")).toString();
    rtn.errorText = query.value(QStringLiteral("error")).toString();
    rtn.recordedDate = query.value(QStringLiteral("recorded_date")).toDateTime();
    rtn.uploadDate = query.value(QStringLiteral("upload_date")).toDateTime();
    rtn.recordedDate.setTimeSpec(Qt::UTC);
    rtn.uploadDate.setTimeSpec(Qt::UTC);
    rtn.tags = getTagsForEvidenceID(evidenceID);
  } else {
    rtn.id = -1;
  }
  return rtn;
}

bool DatabaseConnection::updateEvidenceDescription(const QString &newDescription, qint64 evidenceID)
{
    auto q = executeQuery(_db, QStringLiteral("UPDATE evidence SET description=? WHERE id=?"), {newDescription, evidenceID});
    return (q.lastError().type() == QSqlError::NoError);
}

bool DatabaseConnection::deleteEvidence(qint64 evidenceID)
{
    auto q = executeQuery(_db, QStringLiteral("DELETE FROM evidence WHERE id=?"), {evidenceID});
    return (q.lastError().type() == QSqlError::NoError);
}

bool DatabaseConnection::updateEvidenceError(const QString &errorText, qint64 evidenceID) {
  auto q = executeQuery(_db, QStringLiteral("UPDATE evidence SET error=? WHERE id=?"), {errorText, evidenceID});
  return (q.lastError().type() == QSqlError::NoError);
}

void DatabaseConnection::updateEvidenceSubmitted(qint64 evidenceID) {
  executeQuery(_db, QStringLiteral("UPDATE evidence SET upload_date=datetime('now') WHERE id=?"), {evidenceID});
}

QList<model::Tag> DatabaseConnection::getTagsForEvidenceID(qint64 evidenceID) {
  QList<model::Tag> tags;
  auto getTagQuery = executeQuery(_db, QStringLiteral("SELECT id, tag_id, name FROM tags WHERE evidence_id=?"),
                                  {evidenceID});
  while (getTagQuery.next()) {
    auto tag = model::Tag(getTagQuery.value(QStringLiteral("id")).toLongLong(),
                          getTagQuery.value(QStringLiteral("tag_id")).toLongLong(),
                          getTagQuery.value(QStringLiteral("name")).toString());
    tags.append(tag);
  }
  return tags;
}

QList<model::Tag> DatabaseConnection::getFullTagsForEvidenceIDs(
    const QList<qint64>& evidenceIDs) {
  QList<model::Tag> tags;

  batchQuery(QStringLiteral("SELECT id, evidence_id, tag_id, name FROM tags WHERE evidence_id IN (%1)"), 1, evidenceIDs.size(),
      [evidenceIDs](unsigned int index){
        return QVariantList{evidenceIDs[index]};
      },
      [&tags](const QSqlQuery& resultItem){
        auto tag = model::Tag(resultItem.value(QStringLiteral("id")).toLongLong(),
                              resultItem.value(QStringLiteral("evidence_id")).toLongLong(),
                              resultItem.value(QStringLiteral("tag_id")).toLongLong(),
                              resultItem.value(QStringLiteral("name")).toString());
        tags.append(tag);
      });

  return tags;
}

bool DatabaseConnection::setEvidenceTags(const QList<model::Tag> &newTags, qint64 evidenceID)
{
  if(newTags.isEmpty())
      return false;

  QVariantList newTagIds;
  for (const auto &tag : newTags)
    newTagIds.append(tag.serverTagId);

  auto qDelStr = QStringLiteral("DELETE FROM tags WHERE tag_id NOT IN (?) AND evidence_id = ?");
  auto a = executeQuery(_db, qDelStr, {newTagIds, evidenceID});
  if(a.lastError().type() != QSqlError::NoError)
      return false;

  auto qSelStr = QStringLiteral("SELECT tag_id FROM tags WHERE evidence_id = ?");
  auto currentTagsResult = executeQuery(_db, qSelStr, {evidenceID});
  if (currentTagsResult.lastError().type() != QSqlError::NoError)
      return false;

  QList<qint64> currentTags;
  while (currentTagsResult.next())
    currentTags.append(currentTagsResult.value(QStringLiteral("tag_id")).toLongLong());

  struct dataset {
    qint64 evidenceID = 0;
    qint64 tagID = 0;
    QString name;
  };
  QList<dataset> tagDataToInsert;

  QString baseQuery = QStringLiteral("INSERT INTO tags (evidence_id, tag_id, name) VALUES ");
  for (const auto &newTag : newTags) {
    if (currentTags.count(newTag.serverTagId) == 0) {
      dataset item;
      item.evidenceID = evidenceID;
      item.tagID = newTag.serverTagId;
      item.name = newTag.tagName;
      tagDataToInsert.append(item);
    }
  }

  // one possible concern: we are going to be passing a lot of parameters
  // sqlite indicates it's default is 100 passed parameter, but it can "handle thousands"
  if (!tagDataToInsert.empty()) {
    QVariantList args;
    baseQuery.append(QStringLiteral("(?,?,?)"));
    baseQuery.append(QString(", (?,?,?)").repeated(int(tagDataToInsert.size() - 1)));
    for (const auto &item : tagDataToInsert) {
      args.append(item.evidenceID);
      args.append(item.tagID);
      args.append(item.name);
    }
    auto q = executeQuery(_db, baseQuery, args);
    if (q.lastError().type() != QSqlError::NoError)
        return false;
  }
  return true;
}

void DatabaseConnection::batchCopyTags(const QList<model::Tag> &allTags) {
  QString baseQuery = QStringLiteral("INSERT INTO tags (id, evidence_id, tag_id, name) VALUES %1");
  int varsPerRow = 4;
  std::function<QVariantList(int)> getItemValues = [allTags](int i){
    model::Tag item = allTags.at(i);
    return QVariantList{item.id, item.evidenceId, item.serverTagId, item.tagName};
  };
  batchInsert(baseQuery, varsPerRow, allTags.size(), getItemValues);
}

DBQuery DatabaseConnection::buildGetEvidenceWithFiltersQuery(const EvidenceFilters &filters)
{
  QString query = _sqlSelectTemplate.arg(_evidenceAllKeys, _tblEvidence);
  QVariantList values;
  QStringList parts;

  if (filters.hasError != Tri::Any) {
    parts.append(QStringLiteral(" error LIKE ? "));
    // _% will ensure at least one character exists in the error column, ensuring it's populated
    values.append(filters.hasError == Tri::Yes ? QStringLiteral("_%") : QString());
  }

  if (filters.submitted != Tri::Any) {
    auto sub = QStringLiteral(" upload_data IS%1NULL");
    if(filters.submitted == Tri::Yes)
        parts.append(sub.arg(QStringLiteral(" NOT ")));
    else
        parts.append(sub.arg(QStringLiteral(" ")));
  }

  if (!filters.operationSlug.isEmpty()) {
    parts.append(" operation_slug = ? ");
    values.append(filters.operationSlug);
  }
  if (!filters.contentType.isEmpty()) {
    parts.append(" content_type = ? ");
    values.append(filters.contentType);
  }
  if (filters.startDate.isValid()) {
    parts.append(" recorded_date >= ? ");
    values.append(filters.startDate);
  }
  if (filters.endDate.isValid()) {
    auto realEndDate = filters.endDate.addDays(1);
    parts.append(" recorded_date < ? ");
    values.append(realEndDate);
  }

  if (!parts.empty()) {
    query.append(QStringLiteral(" WHERE %1").arg(parts.at(0)));
    for (size_t i = 1; i < parts.size(); i++)
      query.append(QStringLiteral(" AND %1").arg(parts.at(i)));
  }
  return DBQuery(query, values);
}

void DatabaseConnection::updateEvidencePath(const QString& newPath, qint64 evidenceID)
{
    executeQuery(_db, QStringLiteral("UPDATE evidence SET path=? WHERE id=?"), {newPath, evidenceID});
}

QList<model::Evidence> DatabaseConnection::getEvidenceWithFilters(const EvidenceFilters &filters)
{
    auto dbQuery = buildGetEvidenceWithFiltersQuery(filters);
    auto resultSet = executeQuery(_db, dbQuery.query(), dbQuery.values());
    QList<model::Evidence> allEvidence;

    while (resultSet.next()) {
        model::Evidence evi;
        evi.id = resultSet.value(QStringLiteral("id")).toLongLong();
        evi.path = resultSet.value(QStringLiteral("path")).toString();
        evi.operationSlug = resultSet.value(QStringLiteral("operation_slug")).toString();
        evi.contentType = resultSet.value(QStringLiteral("content_type")).toString();
        evi.description = resultSet.value(QStringLiteral("description")).toString();
        evi.errorText = resultSet.value(QStringLiteral("error")).toString();
        evi.recordedDate = resultSet.value(QStringLiteral("recorded_date")).toDateTime();
        evi.uploadDate = resultSet.value(QStringLiteral("upload_date")).toDateTime();
        evi.recordedDate.setTimeSpec(Qt::UTC);
        evi.uploadDate.setTimeSpec(Qt::UTC);
        allEvidence.append(evi);
    }

    return allEvidence;
}

QList<model::Evidence> DatabaseConnection::createEvidenceExportView(
    const QString& pathToExport, const EvidenceFilters& filters, DatabaseConnection *runningDB)
{
    QList<model::Evidence> exportEvidence;
    auto exportViewAction = [runningDB, filters, &exportEvidence](DatabaseConnection exportDB) {
        exportEvidence = runningDB->getEvidenceWithFilters(filters);
        exportDB.batchCopyFullEvidence(exportEvidence);
        QList<qint64> evidenceIds;
        evidenceIds.resize(exportEvidence.size());
        std::transform(exportEvidence.begin(), exportEvidence.end(), evidenceIds.begin(),
                       [](const model::Evidence& e) { return e.id; });
        QList<model::Tag> tags = runningDB->getFullTagsForEvidenceIDs(evidenceIds);
        exportDB.batchCopyTags(tags);
    };
    withConnection(pathToExport, QStringLiteral("exportDB"), exportViewAction);
    return exportEvidence;
}

bool DatabaseConnection::migrateDB()
{
    qInfo() << "Checking database state";
    auto migrationsToApply = DatabaseConnection::getUnappliedMigrations();

    for (const auto &newMigration : migrationsToApply) {
        QFile migrationFile(QStringLiteral("%1/%2").arg(_migrationPath, newMigration));
        if (!migrationFile.open(QFile::ReadOnly))
            return false;
        auto content = QString(migrationFile.readAll());
        migrationFile.close();
        qInfo() << "Applying Migration: " << newMigration;
        auto upScript = extractMigrateUpContent(content);
        executeQuery(_db, upScript);
        executeQuery(_db, _sqlAddAppliedMigration, {newMigration});
    }

    qInfo() << "All migrations applied";
    return true;
}

QStringList DatabaseConnection::getUnappliedMigrations()
{
    QDir migrationsDir(_migrationPath);
    const auto allMigrations = migrationsDir.entryList(QDir::Files, QDir::Name);
    QStringList appliedMigrations;
    QStringList migrationsToApply;

    auto queryResult = executeQueryNoThrow(_db, _sqlSelectTemplate.arg(_migration_name, _tblMigrations));
    QSqlQuery* dbMigrations = &queryResult.query;
    while (queryResult.success && queryResult.query.next())
        appliedMigrations << dbMigrations->value(_migration_name).toString();
    // compare the two list to find gaps
    for (const auto &possibleMigration : allMigrations) {
        if (!possibleMigration.endsWith(QStringLiteral(".sql")))
            continue;
        auto foundIndex = appliedMigrations.indexOf(possibleMigration);
        if (foundIndex == -1)
            migrationsToApply << possibleMigration;
        else
            appliedMigrations.removeAt(foundIndex);
    }
    if (!appliedMigrations.empty()) {
        qWarning() << "Database is in an inconsistent state";
    }
    return migrationsToApply;
}

// extractMigrateUpContent parses the given migration content and retrieves only
// the portion that applies to the "up" / apply logic. The "down" section is ignored.
QString DatabaseConnection::extractMigrateUpContent(const QString &allContent) noexcept
{
    QString upContent;
    const QStringList lines = allContent.split(_newLine);
    for (const QString &line : lines) {
        auto lowerLine = line.trimmed().toLower();
        if (lowerLine == _migrateUp)
            continue;
        else if (lowerLine == _migrateDown)
            break;
        upContent.append(_lineTemplate.arg(line));
    }
    return upContent;
}

// executeQuery simply attempts to execute the given stmt with the passed args. The statement is
// first prepared, and arg placements can be specified with "?"
QSqlQuery DatabaseConnection::executeQuery(const QSqlDatabase& db, const QString &stmt,
                                           const QVariantList &args) {
  auto result = executeQueryNoThrow(db, stmt, args);
  if (!result.success)
    qWarning() << "Error executing Query: " << result.err.text();
  return std::move(result.query);
}

QueryResult DatabaseConnection::executeQueryNoThrow(const QSqlDatabase& db, const QString &stmt,
                                                  const QVariantList &args) noexcept
{
    QSqlQuery query(db);
    if (!query.prepare(stmt))
        return QueryResult(std::move(query));
    for (const auto &arg : args)
        query.addBindValue(arg);
    query.exec();
    return QueryResult(std::move(query));
}

// doInsert is a version of executeQuery that returns the last inserted id, rather than the
// underlying query/response
// Logs then returns -1
qint64 DatabaseConnection::doInsert(const QSqlDatabase& db, const QString &stmt, const QVariantList &args)
{
  auto query = executeQuery(db, stmt, args);
  if(query.lastInsertId() != QVariant())
    return query.lastInsertId().toLongLong();
  return -1;
}

void DatabaseConnection::batchInsert(const QString& baseQuery, unsigned int varsPerRow, unsigned int numRows,
                                     const FieldEncoderFunc& encodeValues, QString rowInsertTemplate) {
  if (rowInsertTemplate.isEmpty()) {
    rowInsertTemplate = "(" + QString("?,").repeated(varsPerRow > 0 ? varsPerRow-1 : 0) + "?),";
  }
  auto noop = [](const QSqlQuery&){};
  batchQuery(baseQuery, varsPerRow, numRows, encodeValues, noop, rowInsertTemplate);
}

void DatabaseConnection::batchQuery(const QString &baseQuery, unsigned int varsPerRow,
                                    unsigned int numRows, const FieldEncoderFunc &encodeValues,
                                    const RowDecoderFunc& decodeRows, QString variableTemplate) {
  unsigned long frameSize = SQLITE_MAX_VARS / varsPerRow;
  auto numFullFrames = numRows / frameSize;
  auto finalRowOffset = numFullFrames * frameSize;
  auto overflow = numRows - finalRowOffset;

  if (variableTemplate.isEmpty()) {
    variableTemplate = QString("?,").repeated(int(varsPerRow));
  }
  int runningRowIndex = 0; // tracks what row is next to be encoded/"inserted"
  /// prepArgString generates a string that looks like ?,?,?, with as many ? as rowInsertTemplate * numRows
  auto prepArgString = [variableTemplate](unsigned int numRows){
    auto inst = variableTemplate.repeated(int(numRows));
    inst.chop(1);
    return inst;
  };
  /// encodeRowValues generates a vector of QVariants to provide to executeQuery
  auto encodeRowValues = [&runningRowIndex, encodeValues](unsigned int numRows){
    QVariantList values;
    for (unsigned int i = 0; i < numRows; i++) {
      auto itemValues = encodeValues(runningRowIndex++); // increment happens after encoding
      values.append(itemValues);
    }
    return values;
  };
  /// runQuery executes the given query, and iterates over the result set
  auto runQuery = [this, decodeRows](const QString &query, const QVariantList& values) {
    auto completedQuery = executeQuery(_db, query, values);
    while (completedQuery.next()) {
      decodeRows(completedQuery);
    }
  };

  // do full frames
  QString fullFrameQuery = baseQuery.arg(prepArgString(frameSize));
  for(unsigned long frameIndex = 0; frameIndex < numFullFrames; frameIndex++) {
    QVariantList values = encodeRowValues(frameSize);
    runQuery(fullFrameQuery, values); // an alternative here: use execBatch, which might slightly hasten results
  }

  // do the remainder
  if (overflow > 0) {
    QString overflowQuery = baseQuery.arg(prepArgString(overflow));
    QVariantList overflowValues = encodeRowValues(overflow);
    runQuery(overflowQuery, overflowValues);
  }
}
