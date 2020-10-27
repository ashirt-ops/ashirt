#ifndef TYPE_STREAMS_H
#define TYPE_STREAMS_H

#include <QDataStream>

#include "models/tag.h"

QDataStream& operator<<(QDataStream& out, const model::Tag& v) {
  out << v.tagName << v.id << v.serverTagId;
  return out;
}

QDataStream& operator>>(QDataStream& in, model::Tag& v) {
  in >> v.tagName;
  in >> v.id;
  in >> v.serverTagId;
  return in;
}

QDataStream& operator<<(QDataStream& out, const std::vector<model::Tag>& v) {
  out << int(v.size());
  for (auto tag : v) {
    out << tag;
  }
  return out;
}

QDataStream& operator>>(QDataStream& in, std::vector<model::Tag>& v) {
  int qty;
  in >> qty;
  v.reserve(qty);
  for(int i = 0; i < qty; i++) {
    model::Tag t;
    in >> t;
    v.push_back(t);
  }
  return in;
}

class TypeStreams {
 public:
  static void registerTypes() {
    qRegisterMetaTypeStreamOperators<model::Tag>("Tag");
    qRegisterMetaTypeStreamOperators<std::vector<model::Tag>>("TagVector");
  }
};

#endif // TYPE_STREAMS_H
