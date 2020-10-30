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
  DatabaseConnection();

  void connect();
  void close() noexcept;

  DBQuery buildGetEvidenceWithFiltersQuery(const EvidenceFilters &filters);

  model::Evidence getEvidenceDetails(qint64 evidenceID);
  std::vector<model::Evidence> getEvidenceWithFilters(const EvidenceFilters &filters);

  qint64 createEvidence(const QString &filepath, const QString &operationSlug,
                        const QString &contentType);

  void updateEvidenceDescription(const QString &newDescription, qint64 evidenceID);
  void updateEvidenceError(const QString &errorText, qint64 evidenceID);
  void updateEvidenceSubmitted(qint64 evidenceID);
  void updateEvidencePath(QString newPath, qint64 evidenceID);
  void setEvidenceTags(const std::vector<model::Tag> &newTags, qint64 evidenceID);

  void deleteEvidence(qint64 evidenceID);

 private:
  QSqlDatabase db;

  void migrateDB();
  QStringList getUnappliedMigrations();

  static QString extractMigrateUpContent(const QString &allContent) noexcept;
  static QSqlQuery executeQuery(QSqlDatabase *db, const QString &stmt,
                                const std::vector<QVariant> &args = {});
  static qint64 doInsert(QSqlDatabase *db, const QString &stmt, const std::vector<QVariant> &args);
};

#endif  // DATABASECONNECTION_H
