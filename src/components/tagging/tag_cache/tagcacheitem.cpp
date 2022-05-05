#include "tagcacheitem.h"

#include <QDateTime>

TagCacheItem::TagCacheItem() { }

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

void TagCacheItem::setTags(QList<dto::Tag> tags) {
  this->tags = tags;
  expiry = now() + defaultExpiryDeltaMs;
}

QList<dto::Tag> TagCacheItem::getTags() {
  return tags;
}
