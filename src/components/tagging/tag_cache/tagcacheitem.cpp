#include "tagcacheitem.h"

#include <QDateTime>

TagCacheItem::TagCacheItem()
{

}

void TagCacheItem::expire() {
  expiry = 0;
}

bool TagCacheItem::isStale() {
  if (expiry - now() <= 0) {
    return true;
  }
  return false;
}

qint64 TagCacheItem::now() {
  return QDateTime::currentMSecsSinceEpoch();
}

void TagCacheItem::setTags(std::vector<dto::Tag> tags) {
  this->tags = tags;
  this->expiry = now() + defaultExpiryDeltaMs;
}

std::vector<dto::Tag> TagCacheItem::getTags() {
  return tags;
}
