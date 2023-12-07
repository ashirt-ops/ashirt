#pragma once

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

// parseJSONList parses a JSON list into a vector of concrete types from a byte[]. If any error
// occurs during parsing, an empty vector is returned
template <typename T>
static QList<T> parseJSONList(QByteArray data, T (*dataToItem)(QJsonObject)) {
  QJsonParseError err;
  QJsonDocument doc = QJsonDocument::fromJson(data, &err);
  if (err.error != QJsonParseError::NoError) {
    return QList<T>();
  }
  QJsonArray arr = doc.array();
  QList<T> list;

  for (const QJsonValue& val : arr) {
    auto item = dataToItem(val.toObject());
    list.append(item);
  }

  return list;
}

// parseJSONItem parses a single item (assumed to be a Json Object) from a byte[]. If any error
// occurs during parsing, an empty object is returned.
template <typename T>
static T parseJSONItem(QByteArray data, std::function<T(QJsonObject)>dataToItem) {
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
