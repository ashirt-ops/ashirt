// Copyright 2020, Verizon Media
// Licensed under the terms of MIT. See LICENSE file in project root for terms.

#ifndef EVIDENCEFILTER_H
#define EVIDENCEFILTER_H

#include <QDate>
#include <QString>
#include <utility>
#include <vector>
#include <QStringList>

enum Tri { Any, Yes, No };

// These represent the standard key for a filter
const QString FILTER_KEY_ERROR = QStringLiteral("err");
const QString FILTER_KEY_SUBMITTED = QStringLiteral("submitted");
const QString FILTER_KEY_TO = QStringLiteral("to");
const QString FILTER_KEY_FROM = QStringLiteral("from");
const QString FILTER_KEY_ON = QStringLiteral("on");
const QString FILTER_KEY_OPERATION = QStringLiteral("op");
const QString FILTER_KEY_CONTENT_TYPE = QStringLiteral("type");

// These represent aliases for standard key for a filter
const QStringList FILTER_KEYS_ERROR = {
    FILTER_KEY_ERROR, QStringLiteral("error")
    , QStringLiteral("failed"), QStringLiteral("fail")
};
const QStringList FILTER_KEYS_SUBMITTED = {FILTER_KEY_SUBMITTED};
const QStringList FILTER_KEYS_TO = {
    FILTER_KEY_TO, QStringLiteral("before")
    , QStringLiteral("til"), QStringLiteral("until")
};
const QStringList FILTER_KEYS_FROM = {FILTER_KEY_FROM, QStringLiteral("after")};
const QStringList FILTER_KEYS_ON = {FILTER_KEY_ON};
const QStringList FILTER_KEYS_OPERATION = {FILTER_KEY_OPERATION, QStringLiteral("operation")};
const QStringList FILTER_KEYS_CONTENT_TYPE = {FILTER_KEY_CONTENT_TYPE, QStringLiteral("contentType")};

class EvidenceFilters {
 public:
  EvidenceFilters();

  static QString standardizeFilterKey(QString key);
  QString toString() const;
  static EvidenceFilters parseFilter(const QString &text);

 public:
  QString operationSlug;
  QString contentType;
  Tri hasError = Any;
  Tri submitted = Any;
  QDate startDate = QDate();
  QDate endDate = QDate();

 public:
  static Tri parseTri(const QString &text);
  static QString triToString(const Tri &tri);

 private:
  static std::vector<std::pair<QString, QString>> tokenizeFilterText(const QString &text);
  static QDate parseDateString(QString text);
  static Tri parseTriFilterValue(const QString &text, bool strict = false);
};

#endif  // EVIDENCEFILTER_H
