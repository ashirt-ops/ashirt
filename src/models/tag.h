#pragma once

#include <QDataStream>
#include <QMetaType>

namespace model {
class Tag {
 public:
  Tag() = default;
  ~Tag() = default;
  Tag(const Tag &) = default;
  Tag& operator=(const Tag&) = default;

  Tag(qint64 id, qint64 evidenceID, qint64 tagId, QString name) : Tag(id, tagId, name) { this->evidenceId = evidenceID; }
  Tag(qint64 id, qint64 tagId, QString name) : Tag(tagId, name) { this->id = id; }
  Tag(qint64 tagId, QString name) : serverTagId(tagId), tagName(name) { }

 public:
  friend QDataStream& operator<<(QDataStream& out, const model::Tag& v) {
    out << v.tagName << v.id << v.serverTagId;
    return out;
  }

  friend QDataStream& operator>>(QDataStream& in, model::Tag& v) {
    in >> v.tagName;
    in >> v.id;
    in >> v.serverTagId;
    return in;
  }

 public:
  qint64 id;
  qint64 serverTagId;
  QString tagName;
  qint64 evidenceId;
};
}  // namespace model
Q_DECLARE_METATYPE(model::Tag)
