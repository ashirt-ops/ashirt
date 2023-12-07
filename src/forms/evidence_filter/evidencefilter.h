#pragma once

#include <QDate>
#include <QStringList>

enum Tri { Any, Yes, No };

class EvidenceFilters {
 public:
  EvidenceFilters() = default;

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
  static QList<QPair<QString, QString>> tokenizeFilterText(const QString &text);
  static QDate parseDateString(QString text);
  static Tri parseTriFilterValue(const QString &text, bool strict = false);

  // These represent the standard key for a filter
  inline static const QString FILTER_KEY_ERROR = QStringLiteral("err");
  inline static const QString FILTER_KEY_SUBMITTED = QStringLiteral("submitted");
  inline static const QString FILTER_KEY_TO = QStringLiteral("to");
  inline static const QString FILTER_KEY_FROM = QStringLiteral("from");
  inline static const QString FILTER_KEY_ON = QStringLiteral("on");
  inline static const QString FILTER_KEY_OPERATION = QStringLiteral("op");
  inline static const QString FILTER_KEY_CONTENT_TYPE = QStringLiteral("type");

  // These represent aliases for standard key for a filter
  inline static const QStringList FILTER_KEYS_ERROR = {
      FILTER_KEY_ERROR , QStringLiteral("error")
      , QStringLiteral("failed") , QStringLiteral("fail")
  };
  inline static const QStringList FILTER_KEYS_SUBMITTED = {FILTER_KEY_SUBMITTED};
  inline static const QStringList FILTER_KEYS_TO = {
      FILTER_KEY_TO, QStringLiteral("before")
      , QStringLiteral("til"), QStringLiteral("until")
  };
  inline static const QStringList FILTER_KEYS_FROM = {FILTER_KEY_FROM, QStringLiteral("after")};
  inline static const QStringList FILTER_KEYS_ON = {FILTER_KEY_ON};
  inline static const QStringList FILTER_KEYS_OPERATION = {FILTER_KEY_OPERATION, QStringLiteral("operation")};
  inline static const QStringList FILTER_KEYS_CONTENT_TYPE = {FILTER_KEY_CONTENT_TYPE, QStringLiteral("contentType")};
};
