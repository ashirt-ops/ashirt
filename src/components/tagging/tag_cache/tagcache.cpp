#include "tagcache.h"

#include "helpers/netman.h"
#include "helpers/stopreply.h"


TagCache::TagCache() {

}

TagCache::~TagCache() {
  for (auto& entry : tagRequests) {
    stopReply(&(entry.second));
  }
}

void TagCache::requestExpiry(QString operationSlug) {
  auto entry = cache.find(operationSlug);
  if (entry != cache.end()) {
    entry->second.expire();
  }
}

void TagCache::requestTags(QString operationSlug) {
  auto entry = cache.find(operationSlug);

  if (entry == cache.end() || entry->second.isStale()) { // not found/expired
    if (tagRequests.find(operationSlug) != tagRequests.end()) { // message is in progress -- ignore this request
      return;
    }

    auto reply = NetMan::getInstance().getOperationTags(operationSlug);
    tagRequests.emplace(operationSlug, reply);
    connect(reply, &QNetworkReply::finished, [this, reply, operationSlug]() {
      onGetTagsComplete(reply, operationSlug);
      tagRequests.erase(operationSlug);

      // if successful, alert that new tags are ready!
      auto newEntry = cache.find(operationSlug);
      if (newEntry != cache.end()) {
        if (newEntry->second.isStale()) { // lookup failed -- notify with old data
          Q_EMIT failedLookup(operationSlug, newEntry->second.getTags());
        }
        else {
          Q_EMIT tagResponse(operationSlug, newEntry->second.getTags());
        }
      }
      else { // lookup failed, but no data in this scenario
        Q_EMIT failedLookup(operationSlug);
      }
    });
  }
  else { // we already have valid data
    Q_EMIT tagResponse(operationSlug, entry->second.getTags());
  }
}

void TagCache::onGetTagsComplete(QNetworkReply* reply, QString operationSlug) {
  bool isValid;
  auto data = NetMan::extractResponse(reply, isValid);

  if (isValid) {
    std::vector<dto::Tag> tags = dto::Tag::parseDataAsList(data);
    auto item = TagCacheItem();
    item.setTags(tags);
    cache[operationSlug] = item;
  }
  else {
    if (cache.find(operationSlug) == cache.end()) {

    }
  }

  tidyReply(&reply);
}
