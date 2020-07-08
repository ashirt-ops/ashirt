// Copyright 2020, Verizon Media
// Licensed under the terms of GPLv3. See LICENSE file in project root for terms.

#ifndef MODEL_TAG_H
#define MODEL_TAG_H

#include <QString>
#include <QVariant>
#include <QDataStream>

namespace model {
class Tag {
 public:
  Tag() = default;
  ~Tag() = default;
  Tag(const Tag &) = default;

  Tag(qint64 id, qint64 tagId, QString name) : Tag(tagId, name) { this->id = id; }
  Tag(qint64 tagId, QString name) {
    this->serverTagId = tagId;
    this->tagName = name;
  }

 public:
  qint64 id;
  qint64 serverTagId;
  QString tagName;
};
}  // namespace model

Q_DECLARE_METATYPE(model::Tag);
Q_DECLARE_METATYPE(std::vector<model::Tag>);
#endif  // MODEL_TAG_H
