#include "tagcache.h"

#include <QNetworkReply>

#include "helpers/netman.h"
#include "helpers/cleanupreply.h"


TagCache::TagCache(QObject *parent): QObject(parent) {

}

TagCache::~TagCache() {
  for (auto entry : qAsConst(tagRequests)) {
    cleanUpReply(&(entry));
  }
}

void TagCache::requestExpiry(QString operationSlug) {
  auto entry = cache.find(operationSlug);
  if (entry != cache.end()) {
    entry.value().expire();
  }
}

void TagCache::requestTags(QString operationSlug) {
  auto entry = cache.find(operationSlug);

  if (entry == cache.end() || entry->isStale()) { // not found/expired
    if (tagRequests.find(operationSlug) != tagRequests.end()) { // message is in progress -- ignore this request
      return;
    }

    auto reply = NetMan::getOperationTags(operationSlug);
    tagRequests.insert(operationSlug, reply);
    connect(reply, &QNetworkReply::finished, this, [this, reply, operationSlug]() {
      onGetTagsComplete(reply, operationSlug);
      tagRequests.remove(operationSlug);

      // if successful, alert that new tags are ready!
      auto newEntry = cache.find(operationSlug);
      if (newEntry != cache.end()) {
        if (newEntry->isStale()) { // lookup failed -- notify with old data
          Q_EMIT failedLookup(operationSlug, newEntry->getTags());
        }
        else {
          Q_EMIT tagResponse(operationSlug, newEntry->getTags());
        }
      }
      else { // lookup failed, but no data in this scenario
        Q_EMIT failedLookup(operationSlug);
      }
    });
  }
  else { // we already have valid data
    Q_EMIT tagResponse(operationSlug, entry->getTags());
  }
}

void TagCache::onGetTagsComplete(QNetworkReply* reply, QString operationSlug) {
  bool isValid;
  auto data = NetMan::extractResponse(reply, isValid);

  if (isValid) {
    QList<dto::Tag> tags = dto::Tag::parseDataAsList(data);
    auto item = TagCacheItem();
    item.setTags(tags);
    cache[operationSlug] = item;
  }
  else {
    if (cache.find(operationSlug) == cache.end()) {

    }
  }

  cleanUpReply(&reply);
}
