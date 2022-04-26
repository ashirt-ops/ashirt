#pragma once

#include "dtos/tag.h"

class TagCacheItem
{
 public:
  TagCacheItem();

 public:
  void expire();
  bool isStale();
  void setTags(std::vector<dto::Tag> tags);
  std::vector<dto::Tag> getTags();

 private:
  qint64 now();

 private:
  static const qint64 defaultExpiryDeltaMs = 60*1000;
  qint64 expiry;
  std::vector<dto::Tag> tags;
};
