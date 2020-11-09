// Copyright 2020, Verizon Media
// Licensed under the terms of MIT. See LICENSE file in project root for terms.

#ifndef DATABASECONNECTION_H
#define DATABASECONNECTION_H

#include <QSqlDatabase>
#include <QSqlDriver>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>
#include <QString>
#include <QVariant>

#include "forms/evidence_filter/evidencefilter.h"
#include "models/evidence.h"
#include "helpers/constants.h"
#include "db/query_result.h"

class DBQuery {
 private:
  QString _query;
  std::vector<QVariant> _values;

 public:
  DBQuery(QString query) : DBQuery(query, {}) {}
  DBQuery(QString query, std::vector<QVariant> values) {
    this->_query = query;
    this->_values = values;
  }
  inline QString query() { return _query; }
  inline std::vector<QVariant> values() { return _values; }
};

class DatabaseConnection {
 public:
  DatabaseConnection(QString dbPath, QString databaseName);

  /**
   * @brief withConnection acts as a context manager for a single database connection. The goal for
   * this method is for interaction with secondary databases (e.g. during import/export), where
   * interaction is localized and infrequent.
   * @param dbPath The path to the database file
   * @param dbName The name of the database connection (can be anything, but should be unique.
   * Specifically, it should NOT be the value from Constants::databaseName())
   * @param actions A function that will execute after a connection is established. This is where
   * all db interactions should occur.
   */
  static void withConnection(QString dbPath, QString dbName, std::function<void(DatabaseConnection)>actions);

  void connect();
  void close() noexcept;

  static DBQuery buildGetEvidenceWithFiltersQuery(const EvidenceFilters &filters);

  model::Evidence getEvidenceDetails(qint64 evidenceID);
  std::vector<model::Evidence> getEvidenceWithFilters(const EvidenceFilters &filters);

  qint64 createEvidence(const QString &filepath, const QString &operationSlug,
                        const QString &contentType);
  qint64 createFullEvidence(const model::Evidence &evidence);
  qint64 createFullEvidenceWithID(const model::Evidence &evidence);

  void updateEvidenceDescription(const QString &newDescription, qint64 evidenceID);
  void updateEvidenceError(const QString &errorText, qint64 evidenceID);
  void updateEvidenceSubmitted(qint64 evidenceID);
  void updateEvidencePath(QString newPath, qint64 evidenceID);
  void setEvidenceTags(const std::vector<model::Tag> &newTags, qint64 evidenceID);

  void deleteEvidence(qint64 evidenceID);

  /// createEvidenceExportView duplicates the normal database with only a subset of evidence present,
  /// as well as related data (e.g. tags)
  ///
  /// Note that currently, this simply exports everything. This is included as a way to limit sharing
  /// in the future.
  static std::vector<model::Evidence> createEvidenceExportView(QString pathToExport, EvidenceFilters filters, DatabaseConnection* runningDB);
  std::vector<model::Tag> getTagsForEvidenceID(qint64 evidenceID);

 private:
  QString dbName = "";

  void migrateDB();
  QSqlDatabase getDB();

  static QStringList getUnappliedMigrations(const QSqlDatabase &db);
  static QString extractMigrateUpContent(const QString &allContent) noexcept;
  static QSqlQuery executeQuery(const QSqlDatabase& db, const QString &stmt,
                                const std::vector<QVariant> &args = {});

  /// executeQueryNoThrow provides a safe mechanism to execute a query on the database. (Safe in the
  /// sense that no exception is thrown). It is incumbent on the caller to inspect the
  /// QueryResult.sucess/QueryResult.err fields to determine the actual result.
  static QueryResult executeQueryNoThrow(const QSqlDatabase& db, const QString &stmt,
                                const std::vector<QVariant> &args = {}) noexcept;
  static qint64 doInsert(const QSqlDatabase &db, const QString &stmt, const std::vector<QVariant> &args);
};

#endif  // DATABASECONNECTION_H
