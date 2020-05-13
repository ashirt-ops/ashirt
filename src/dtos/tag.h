// Copyright 2020, Verizon Media
// Licensed under the terms of GPLv3. See LICENSE file in project root for terms.

#ifndef DTO_TAG_H
#define DTO_TAG_H

#include <QVariant>
#include <vector>

#include "helpers/jsonhelpers.h"

namespace dto {
class Tag {
 public:
  Tag() {}
  Tag(QString name, QString colorName) {
    this->name = name;
    this->colorName = colorName;
  }

  qint64 id;
  QString colorName;
  QString name;

  static Tag parseData(QByteArray data) { return parseJSONItem<Tag>(data, Tag::fromJson); }

  static std::vector<Tag> parseDataAsList(QByteArray data) {
    return parseJSONList<Tag>(data, Tag::fromJson);
  }

  static QByteArray toJson(Tag t) {
    QJsonObject obj;
    obj.insert("colorName", t.colorName);
    obj.insert("name", t.name);
    return QJsonDocument(obj).toJson();
  }

 private:
  // provides a Tag from a given QJsonObject
  static Tag fromJson(QJsonObject obj) {
    Tag t;
    t.id = obj["id"].toVariant().toLongLong();
    t.colorName = obj["colorName"].toString();
    t.name = obj["name"].toString();

    return t;
  }
};
}  // namespace dto

#endif  // DTO_TAG_H
