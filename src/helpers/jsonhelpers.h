// Copyright 2020, Verizon Media
// Licensed under the terms of MIT. See LICENSE file in project root for terms.

#ifndef JSONHELPERS_H
#define JSONHELPERS_H

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <vector>

// parseJSONList parses a JSON list into a vector of concrete types from a byte[]. If any error
// occurs during parsing, an empty vector is returned
template <typename T>
static std::vector<T> parseJSONList(QByteArray data, T (*dataToItem)(QJsonObject)) {
  QJsonParseError err;
  QJsonDocument doc = QJsonDocument::fromJson(data, &err);
  if (err.error != QJsonParseError::NoError) {
    return std::vector<T>();
  }
  QJsonArray arr = doc.array();
  std::vector<T> list;

  for (QJsonValue val : arr) {
    auto item = dataToItem(val.toObject());
    list.push_back(item);
  }

  return list;
}

// parseJSONItem parses a single item (assumed to be a Json Object) from a byte[]. If any error
// occurs during parsing, an empty object is returned.
template <typename T>
static T parseJSONItem(QByteArray data, T (*dataToItem)(QJsonObject)) {
  QJsonParseError err;
  QJsonDocument doc = QJsonDocument::fromJson(data, &err);
  if (err.error != QJsonParseError::NoError) {
    return T();
  }
  return dataToItem(doc.object());
}

// Possilbe generic version of converting to json
// template <typename T>
// static QByteArray toJSONObject(T item, QJsonObject(*itemToData)(T)) {
//    auto obj = itemToData(item);
//    return QJsonDocument::fromVariant(obj).toJson();
//}

#endif  // JSONHELPERS_H
