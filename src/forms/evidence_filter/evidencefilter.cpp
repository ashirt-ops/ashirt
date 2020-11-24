// Copyright 2020, Verizon Media
// Licensed under the terms of MIT. See LICENSE file in project root for terms.

#include "evidencefilter.h"

EvidenceFilters::EvidenceFilters() = default;

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
  if (FILTER_KEYS_SERVER_UUID.contains(key, Qt::CaseInsensitive)) {
    return FILTER_KEY_SERVER_UUID;
  }
  return key;
}

QString EvidenceFilters::toString() const {
  // helper functions
  static auto toCommonDate = [](QDate d) -> QString {
    auto today = QDateTime::currentDateTime().date();
    if (d == today) {
      return "Today";
    }
    if (d == today.addDays(-1)) {
      return "Yesterday";
    }
    return d.toString("yyyy-MM-dd");
  };
  static auto triToText = [](Tri t) -> QString { return t == Yes ? "yes" : "no"; };

  QString rtn = "";
  if (!operationSlug.isEmpty()) {
    rtn.append(" " + FILTER_KEY_OPERATION + ": " + operationSlug);
  }
  if (!contentType.isEmpty()) {
    rtn.append(" " + FILTER_KEY_CONTENT_TYPE + ": " + contentType);
  }
  if (hasError != Any) {
    rtn.append(" " + FILTER_KEY_ERROR + ": " + triToText(hasError));
  }
  if (startDate.isValid() || endDate.isValid()) {
    if (startDate == endDate) {
      rtn.append(" " + FILTER_KEY_ON + ": " + toCommonDate(startDate));
    }
    else {
      if (startDate.isValid()) {
        rtn.append(" " + FILTER_KEY_FROM + ": " + toCommonDate(startDate));
      }

      if (endDate.isValid()) {
        rtn.append(" " + FILTER_KEY_TO + ": " + toCommonDate(endDate));
      }
    }
  }
  if (submitted != Any) {
    rtn.append(" " + FILTER_KEY_SUBMITTED + ": " + triToText(submitted));
  }
  if (!serverUuid.isEmpty()) {
    rtn.append(" " + FILTER_KEY_SERVER_UUID + ": " + serverUuid);
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
    else if (key == FILTER_KEY_SERVER_UUID) {
      filter.serverUuid = value;
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
  if (val.startsWith("t") || val.startsWith("y")) {
    return Tri::Yes;
  }
  if (strict) {
    return (val.startsWith("f") || val.startsWith("n")) ? Tri::No : Tri::Any;
  }
  return Tri::No;
}

std::vector<std::pair<QString, QString>> EvidenceFilters::tokenizeFilterText(const QString& text) {
  bool inQuote = false;
  int startWordIndex = 0;
  std::vector<std::pair<QString, QString>> rtn;

  bool hasValue = false;
  int sepLocation = -1;

  auto maybeAddWord = [&rtn, &hasValue, text](int startLocation, int sepLocation, int endLocation){
    hasValue = false;

    if (sepLocation == -1) {
      return;
    }
    QString key = text.mid(startLocation, sepLocation - startLocation).trimmed().replace("\"", "");
    QString value = text.mid(sepLocation+1, endLocation - sepLocation).trimmed().replace("\"", "");
    if (key == "" || value == "") {
      return;
    }
    QString adjustedPhrase = key + ":" + value;

    rtn.push_back(std::pair<QString, QString>(key, value));
  };

  for(int i = 0; i < text.size(); i++) {
    QChar ch = text.at(i);
    bool addWord = false;

    if(sepLocation > -1 && !ch.isSpace()) {
      hasValue = true;
    }

    if(ch == "\"") {
      inQuote = !inQuote;
      if(inQuote == false) {
        addWord = true;
      }
    }
    else if(ch == ":" && !inQuote && sepLocation == -1) {
      sepLocation = i;
    }
    else if(ch.isSpace() && !inQuote) {
      addWord = true;
    }

    if(addWord && hasValue) {
      maybeAddWord(startWordIndex, sepLocation, i);
      sepLocation = -1;
      startWordIndex = i+1;
    }
  }
  maybeAddWord(startWordIndex, sepLocation, text.length()-1);

  return rtn;
}

QDate EvidenceFilters::parseDateString(QString text) {
  static QString DATE_FORMAT = "yyyy-MM-dd";

  text = text.toLower();
  if (text == "today") {
    text = QDateTime::currentDateTime().toString(DATE_FORMAT);
  }
  else if (text == "yesterday") {
    text = QDateTime::currentDateTime().addDays(-1).toString(DATE_FORMAT);
  }
  // other thoughts: this week (hard -- spans years), this month, this year, last ${period}

  return QDate::fromString(text, DATE_FORMAT);
}

// parseTri returns Tri::Yes if the given text is exactly "Yes", Tri::No if the text is exactly "No"
// otherwise Tri::Any.
// This is the inverse of triToString
Tri EvidenceFilters::parseTri(const QString& text) {
  if (text == "Yes") {
    return Yes;
  }
  if (text == "No") {
    return No;
  }
  return Any;
}

// triToString returns "Yes" for Tri::Yes, "No" for Tri::No, otherwise "Any"
// This is the inverse to parseTri
QString EvidenceFilters::triToString(const Tri& tri) {
  switch (tri) {
    case Yes:
      return "Yes";
    case No:
      return "No";
    default:
      return "Any";
  }
}

// once unit tests are available, the should serve as a good starting point

//bool matches(std::vector<std::pair<QString, QString>> a, std::vector<std::pair<QString, QString>> b, QString label="") {
//  if (label != "") {
//    std::cout << "[" << label.toStdString() << "]";
//  }

//  if (a.size() != b.size()) {
//    std::cout << "No match: size difference" << std::endl;
//    return false;
//  }

//  for(size_t i = 0; i < a.size(); i++) {
//    auto expected = a[i];
//    auto actual = b[i];
//    if (expected.first != actual.first || expected.second != actual.second) {
//      std::cout << "No match: " << expected.first.toStdString() << "!=" << actual.first.toStdString()
//                << " || " << expected.second.toStdString() << "!=" << actual.second.toStdString()
//                << std::endl;
//      return false;
//    }
//  }

//  std::cout << "okay!" << std::endl;

//  return true;
//}

//void unitTestsForTokenize() {
//  auto mkpair = [](QString a, QString b){
//    return std::pair<QString,QString>(a, b);
//  };
//  // existing cases that should work
//  matches(tokenizeFilterText("a:b"), {mkpair("a", "b")});
//  matches(tokenizeFilterText("a:b c:d"), {mkpair("a", "b"), mkpair("c", "d")});
//  matches(tokenizeFilterText("a:b  c:d"), {mkpair("a", "b"), mkpair("c", "d")});
//  matches(tokenizeFilterText("a:b c:d "), {mkpair("a", "b"), mkpair("c", "d")});
//  matches(tokenizeFilterText("a : b c : d"), {mkpair("a", "b"), mkpair("c", "d")}); // no good
//  matches(tokenizeFilterText("a :b c :d"), {mkpair("a", "b"), mkpair("c", "d")}); // no good
//  matches(tokenizeFilterText("a: b c: d"), {mkpair("a", "b"), mkpair("c", "d")}); // no good

//  // targets
//  matches(tokenizeFilterText("name:\"joel smith\" age:38"), {mkpair("name", "joel smith"), mkpair("age", "38")});
//  matches(tokenizeFilterText("name:\"joel smith\"age:38"), {mkpair("name", "joel smith"), mkpair("age", "38")});
//  matches(tokenizeFilterText("name:\"joel : smith\" age:38"), {mkpair("name", "joel : smith"), mkpair("age", "38")});
//  matches(tokenizeFilterText("\"name\":\"joel smith\" age:38"), {mkpair("name", "joel smith"), mkpair("age", "38")}); // no good
//}
