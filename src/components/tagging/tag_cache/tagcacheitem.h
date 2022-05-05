#pragma once

#include "dtos/tag.h"

class TagCacheItem
{
 public:
  TagCacheItem();

 public:
  void expire();
  bool isStale();
  void setTags(QList<dto::Tag> tags);
  QList<dto::Tag> getTags();

 private:
  qint64 now();

 private:
  inline static const qint64 defaultExpiryDeltaMs = 60000;
  qint64 expiry;
  QList<dto::Tag> tags;
};
