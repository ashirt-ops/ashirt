// Copyright 2020, Verizon Media
// Licensed under the terms of MIT. See LICENSE file in project root for terms.

#pragma once

#include <QVariant>
#include <vector>

#include "helpers/jsonhelpers.h"
#include "models/tag.h"

namespace dto {
class Tag {
 public:
  Tag() = default;
  ~Tag() = default;
  Tag(const Tag &) = default;

  Tag(QString name, QString colorName) {
    this->name = name;
    this->colorName = colorName;
  }

 public:
  static Tag parseData(QByteArray data) { return parseJSONItem<Tag>(data, Tag::fromJson); }

  static std::vector<Tag> parseDataAsList(QByteArray data) {
    return parseJSONList<Tag>(data, Tag::fromJson);
  }

  static QByteArray toJson(Tag t) {
    QJsonObject obj;
    obj.insert(QStringLiteral("colorName"), t.colorName);
    obj.insert(QStringLiteral("name"), t.name);
    return QJsonDocument(obj).toJson();
  }

  static Tag fromModelTag(model::Tag tag, QString colorName) {
    Tag t;
    t.name = tag.tagName;
    t.colorName = colorName;
    return t;
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

 public:
  qint64 id;
  QString colorName;
  QString name;

};
}  // namespace dto
Q_DECLARE_METATYPE(dto::Tag);
