// Copyright 2020, Verizon Media
// Licensed under the terms of MIT. See LICENSE file in project root for terms.

#ifndef MODEL_TAG_H
#define MODEL_TAG_H

#include <QString>

namespace model {
class Tag {
 public:
  Tag(qint64 id, qint64 tagId, QString name) : Tag(tagId, name) { this->id = id; }
  Tag(qint64 tagId, QString name) {
    this->serverTagId = tagId;
    this->tagName = name;
  }

  qint64 id;
  qint64 serverTagId;
  QString tagName;
};
}  // namespace model

#endif  // MODEL_TAG_H
