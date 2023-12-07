#pragma once

#include <QSqlDatabase>
#include <QSqlDriver>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>

#include "forms/evidence_filter/evidencefilter.h"
#include "models/evidence.h"
#include "helpers/constants.h"
#include "query_result.h"

using FieldEncoderFunc = std::function<QVariantList(unsigned int)>;
using RowDecoderFunc = std::function<void(const QSqlQuery&)>;

class DBQuery {
 private:
  QString _query;
  QVariantList _values;

 public:
  DBQuery(QString query) : DBQuery(query, {}) {}
  DBQuery(QString query, QVariantList values) :_query(query), _values(values) { }
  inline QString query() { return _query; }
  inline QVariantList values() { return _values; }
};
/**
 * @brief The DatabaseConnection class Interface to the local database
 * All Changes / reads to db should return true on success
 * any failed actions can have erorrs checked with DatabaseConnection::errorString()
 */
class DatabaseConnection {
 public:
  const unsigned long SQLITE_MAX_VARS = 999;
  QString getDatabasePath() { return _dbPath; }
   /**
   * @brief DatabaseConnection construct a connect to the database,
   * Uses QSQLite Driver if the driver is missing the app to exit with code 255.
   * @param dbPath - Path to the database
   * @param databaseName - Name of the databaseFile or defaultName if none provided
   */
  DatabaseConnection(const QString& dbPath, const QString& databaseName = Constants::defaultDbName);

  /**
   * @brief withConnection acts as a context manager for a single database connection. The goal for
   * this method is for interaction with secondary databases (e.g. during import/export), where
   * interaction is localized and infrequent.
   * @param dbPath The path to the database file
   * @param dbName The name of the database connection (can be anything, but should be unique.
   * Specifically, it should NOT be the value from Constants::databaseName)
   * @param actions A function that will execute after a connection is established. This is where
   * all db interactions should occur.
   * Returns True is successful
   */
  static bool withConnection(const QString& dbPath, const QString &dbName,
                             const std::function<void(DatabaseConnection)> &actions);

  ///Return the last Error
  QString errorString() {return _db.lastError().text();}
  bool connect();
  void close() noexcept {_db.close();}

  static DBQuery buildGetEvidenceWithFiltersQuery(const EvidenceFilters &filters);

  model::Evidence getEvidenceDetails(qint64 evidenceID);
  QList<model::Evidence> getEvidenceWithFilters(const EvidenceFilters &filters);

  /// Return -1 if Failed
  qint64 createEvidence(const QString &filepath, const QString &operationSlug,
                        const QString &contentType);
  qint64 createFullEvidence(const model::Evidence &evidence);
  void batchCopyFullEvidence(const QList<model::Evidence> &evidence);
  qint64 copyFullEvidence(const model::Evidence &evidence);

  /**
  * @brief updateEvidenceDescription
  * @param newDescription
  * @param evidenceID
  * @return True if successful
  */
  bool updateEvidenceDescription(const QString &newDescription, qint64 evidenceID);
  bool updateEvidenceError(const QString &errorText, qint64 evidenceID);
  void updateEvidenceSubmitted(qint64 evidenceID);
  void updateEvidencePath(const QString& newPath, qint64 evidenceID);
  bool setEvidenceTags(const QList<model::Tag> &newTags, qint64 evidenceID);
  void batchCopyTags(const QList<model::Tag> &allTags);
  QList<model::Tag> getFullTagsForEvidenceIDs(const QList<qint64>& evidenceIDs);

  /**
   * @brief deleteEvidence Delete Evidence from the database
   * @param evidenceID - ID To Delete
   * @return true if successful
   */
  bool deleteEvidence(qint64 evidenceID);

  /// createEvidenceExportView duplicates the normal database with only a subset of evidence
  /// present, as well as related data (e.g. tags)
  ///
  /// Note that currently, this simply exports everything. This is included as a way to limit
  /// sharing in the future.
  static QList<model::Evidence> createEvidenceExportView(const QString& pathToExport,
                                                               const EvidenceFilters& filters,
                                                               DatabaseConnection *runningDB);
  QList<model::Tag> getTagsForEvidenceID(qint64 evidenceID);

  QSqlError lastError() {return _db.lastError();}

 private:
  QString _dbName;
  QString _dbPath;
  QSqlDatabase _db = QSqlDatabase();
  inline static const auto _migrateUp = QStringLiteral("-- +migrate up");
  inline static const auto _migrateDown = QStringLiteral("-- +migrate down");
  inline static const auto _newLine = QStringLiteral("\n");
  inline static const auto _lineTemplate = QStringLiteral("%1").append(_newLine);
  inline static const auto _migrationPath = QStringLiteral(":/migrations");
  inline static const auto _sqlSelectTemplate = QStringLiteral("SELECT %1 FROM %2");
  inline static const auto _sqlBasicInsert = QStringLiteral("INSERT INTO %1 (%2) VALUES (%3)");
  inline static const auto _sqlAddAppliedMigration = QStringLiteral("INSERT INTO migrations (migration_name, applied_at) VALUES (?, datetime('now'))");
  inline static const auto _migration_name = QStringLiteral("migration_name");
  inline static const auto _tblEvidence = QStringLiteral("evidence");
  inline static const auto _tblMigrations = QStringLiteral("migrations");
  inline static const auto _evidenceAllKeys = QStringLiteral("id, path, operation_slug, content_type, description, error, recorded_date, upload_date");

  /**
   * @brief migrateDB - Check migration status and apply any outstanding ones
   * @return true if successful
   */
  bool migrateDB();

  /**
   * @brief getUnappliedMigrations retrieves a list of all of the migrations that have not been applied to the database db
   * Note: only files ending in ".sql" are checked
   * @return List of migrations that have not be applied
   */
  QStringList getUnappliedMigrations();
  QString extractMigrateUpContent(const QString &allContent) noexcept;
  static QSqlQuery executeQuery(const QSqlDatabase& db, const QString &stmt,
                                const QVariantList &args = {});

  /// executeQueryNoThrow provides a safe mechanism to execute a query on the database. (Safe in the
  /// sense that no exception is thrown). It is incumbent on the caller to inspect the
  /// QueryResult.sucess/QueryResult.err fields to determine the actual result.
  static QueryResult executeQueryNoThrow(const QSqlDatabase& db, const QString &stmt,
                                const QVariantList &args = {}) noexcept;

  /**
   * @brief doInsert is a version of executeQuery that returns the last inserted id, rather than the underlying query/response
   * @param db database to act upon
   * @param stmt sql to run
   * @param args args
   * @return Inserted ID or -1 if failed.
   */
  static qint64 doInsert(const QSqlDatabase &db, const QString &stmt, const QVariantList &args);

  /**
   * @brief batchInsert batches multiple inserts over as few requests as possible.
   * @param baseQuery The insert string, up to " VALUES "
   * @param varsPerRow the number of items per row
   * @param numRows the number of rows you wish to insert
   * @param encodeValues A function that, given a row index, will return a QVariantList with each column's data for that row
   * @param rowInsertTemplate An optional string that can be used to define each row's values. Defaults to (?, ..., ?)
   */
  void batchInsert(const QString& baseQuery, unsigned int varsPerRow, unsigned int numRows,
                   const FieldEncoderFunc& encodeValues, QString rowInsertTemplate = QString());

  /**
   * @brief batchQuery batches a query with many variables into as few queries as possible.
   * Note: All variables need to be in the same location in the query.
   * @param baseQuery A QString template query, with the variables denoted with "%1"
   * @param varsPerRow The number of variables per requested (expected: 1)
   * @param numRows the number of variables to placve
   * @param encodeValues A function that, given an index, returns a QVariantList for each variable group
   * @param decodeRows A function that can be used to retrieve the rows from the result set
   * @param variableTemplate An optional string that can be used to define how variables are handled. Defaults to ?,...,?
   */
  void batchQuery(const QString &baseQuery, unsigned int varsPerRow, unsigned int numRows,
                  const FieldEncoderFunc &encodeValues, const RowDecoderFunc& decodeRows,
                  QString variableTemplate = QString());
};
