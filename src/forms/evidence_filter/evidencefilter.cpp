#include "evidencefilter.h"

QString EvidenceFilters::standardizeFilterKey(QString key) {
  if (FILTER_KEYS_ERROR.contains(key, Qt::CaseInsensitive)) {
    return FILTER_KEY_ERROR;
  }
  if (FILTER_KEYS_SUBMITTED.contains(key, Qt::CaseInsensitive)) {
    return FILTER_KEY_SUBMITTED;
  }
  if (FILTER_KEYS_TO.contains(key, Qt::CaseInsensitive)) {
    return FILTER_KEY_TO;
  }
  if (FILTER_KEYS_FROM.contains(key, Qt::CaseInsensitive)) {
    return FILTER_KEY_FROM;
  }
  if (FILTER_KEYS_ON.contains(key, Qt::CaseInsensitive)) {
    return FILTER_KEY_ON;
  }
  if (FILTER_KEYS_OPERATION.contains(key, Qt::CaseInsensitive)) {
    return FILTER_KEY_OPERATION;
  }
  if (FILTER_KEYS_CONTENT_TYPE.contains(key, Qt::CaseInsensitive)) {
    return FILTER_KEY_CONTENT_TYPE;
  }
  return key;
}

QString EvidenceFilters::toString() const {
  // helper functions
  static auto toCommonDate = [](QDate d) -> QString {
    auto today = QDateTime::currentDateTime().date();
    if (d == today) {
      return QT_TRANSLATE_NOOP("EvidenceFilter", "Today");
    }
    if (d == today.addDays(-1)) {
      return QT_TRANSLATE_NOOP("EvidenceFilter", "Yesterday");
    }
    return d.toString(QStringLiteral("yyyy-MM-dd"));
  };
  static auto triToText = [](Tri t) -> QString {
      return t == Yes
              ? QT_TRANSLATE_NOOP("EvidenceFilter", "yes")
              : QT_TRANSLATE_NOOP("EvidenceFilter", "no");
  };

  QString rtn;
  auto appendTemp = QStringLiteral(" %1: %2");
  if (!operationSlug.isEmpty()) {
    rtn.append(appendTemp.arg(FILTER_KEY_OPERATION, operationSlug));
  }
  if (!contentType.isEmpty()) {
    rtn.append(appendTemp.arg(FILTER_KEY_CONTENT_TYPE, contentType));
  }
  if (hasError != Any) {
    rtn.append(appendTemp.arg(FILTER_KEY_ERROR, triToText(hasError)));
  }
  if (startDate.isValid() || endDate.isValid()) {
    if (startDate == endDate) {
      rtn.append(appendTemp.arg(FILTER_KEY_ON, toCommonDate(startDate)));
    }
    else {
      if (startDate.isValid()) {
        rtn.append(appendTemp.arg(FILTER_KEY_FROM, toCommonDate(startDate)));
      }

      if (endDate.isValid()) {
        rtn.append(appendTemp.arg(FILTER_KEY_TO, toCommonDate(endDate)));
      }
    }
  }
  if (submitted != Any) {
    rtn.append(appendTemp.arg(FILTER_KEY_SUBMITTED, triToText(submitted)));
  }

  return rtn.trimmed();
}

EvidenceFilters EvidenceFilters::parseFilter(const QString& text) {
  EvidenceFilters filter;
  if (text.trimmed().isEmpty()) {
    return filter;
  }

  auto tokenizedFilter = tokenizeFilterText(text);

  for (const auto& item : tokenizedFilter) {
    QString key = EvidenceFilters::standardizeFilterKey(item.first.toLower().trimmed());
    QString value = item.second.trimmed();

    if (key == FILTER_KEY_ERROR) {
      auto val = value.toLower();
      filter.hasError = parseTriFilterValue(val);
    }
    else if (key == FILTER_KEY_SUBMITTED) {
      auto val = value.toLower();
      filter.submitted = parseTriFilterValue(val);
    }
    else if (key == FILTER_KEY_OPERATION) {
      filter.operationSlug = value;
    }
    else if (key == FILTER_KEY_TO) {
      filter.endDate = parseDateString(value);
    }
    else if (key == FILTER_KEY_FROM) {
      filter.startDate = parseDateString(value);
    }
    else if (key == FILTER_KEY_ON) {
      auto formattedValue = parseDateString(value);
      filter.startDate = formattedValue;
      filter.endDate = formattedValue;
    }
    else if (key == FILTER_KEY_CONTENT_TYPE) {
      filter.contentType = value;
    }
  }

  return filter;
}

// parseTriFilterValue returns a Tri object given a string. If the given string is "t" or "y"
// then Tri::Yes will be returned. Otherwise, in non-strict mode, Tri::No will be returned.
// In strict mode, Tri::No will be returned only if it starts with "f" or "n", otherwise Tri::Any
// is returned.
Tri EvidenceFilters::parseTriFilterValue(const QString& text, bool strict) {
  auto val = text.toLower().trimmed();
  if (val.startsWith(QStringLiteral("t")) || val.startsWith(QStringLiteral("y"))) {
    return Tri::Yes;
  }
  if (strict) {
    return (val.startsWith(QStringLiteral("f")) || val.startsWith(QStringLiteral("n"))) ? Tri::No : Tri::Any;
  }
  return Tri::No;
}

QList<QPair<QString, QString>> EvidenceFilters::tokenizeFilterText(const QString& text) {
  QStringList list = text.split(QStringLiteral(":"), Qt::SkipEmptyParts);
  // now in: [Key][value key]...[value] format
  QStringList keys;
  QStringList values;
  keys.append(list.first());

  for (int i = 1; i < list.size() - 1; i++) {
    auto valueKeyPair = list.at(i).split(QStringLiteral(" "), Qt::SkipEmptyParts);
    keys.append(valueKeyPair.last());
    valueKeyPair.removeLast();
    values.append(valueKeyPair.join(QStringLiteral(" ")));
  }
  values.append(list.last());

  QList<QPair<QString, QString>> rtn;

  for (int i = 0; i < keys.length(); i++) {
    auto keyvalue = QPair<QString, QString>(keys.at(i), values.at(i));
    rtn.append(keyvalue);
  }
  return rtn;
}

QDate EvidenceFilters::parseDateString(QString text) {
  static QString DATE_FORMAT = QStringLiteral("yyyy-MM-dd");

  text = text.toLower();
  if (text == QT_TRANSLATE_NOOP("EvidenceFilter", "today")) {
    text = QDateTime::currentDateTime().toString(DATE_FORMAT);
  }
  else if (text == QT_TRANSLATE_NOOP("EvidenceFilter", "yesterday")) {
    text = QDateTime::currentDateTime().addDays(-1).toString(DATE_FORMAT);
  }
  // other thoughts: this week (hard -- spans years), this month, this year, last ${period}

  return QDate::fromString(text, DATE_FORMAT);
}

// parseTri returns Tri::Yes if the given text is exactly "Yes", Tri::No if the text is exactly "No"
// otherwise Tri::Any.
// This is the inverse of triToString
Tri EvidenceFilters::parseTri(const QString& text) {
  if (text == QT_TRANSLATE_NOOP("EvidenceFilter", "Yes")) {
    return Yes;
  }
  if (text == QT_TRANSLATE_NOOP("EvidenceFilter", "No")) {
    return No;
  }
  return Any;
}

// triToString returns "Yes" for Tri::Yes, "No" for Tri::No, otherwise "Any"
// This is the inverse to parseTri
QString EvidenceFilters::triToString(const Tri& tri) {
  switch (tri) {
    case Yes:
      return QT_TRANSLATE_NOOP("EvidenceFilter", "Yes");
    case No:
      return QT_TRANSLATE_NOOP("EvidenceFilter", "No");
    default:
      return QT_TRANSLATE_NOOP("EvidenceFilter", "Any");
  }
}
