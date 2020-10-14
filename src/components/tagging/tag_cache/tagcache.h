#ifndef TAGCACHE_H
#define TAGCACHE_H

#include <QString>
#include <QNetworkReply>
#include <unordered_map>

#include "tagcacheitem.h"
#include "dtos/tag.h"


class TagCache : public QObject {
  Q_OBJECT
 public:
  TagCache();
  ~TagCache();

 public:
 signals:
  void tagResponse(QString operationSlug, std::vector<dto::Tag> tags);
  void failedLookup(QString operationSlug, std::vector<dto::Tag> oldTags=std::vector<dto::Tag>());

 private slots:
  void onGetTagsComplete(QNetworkReply* reply, QString operationSlug);

 public:
  void requestTags(QString operationSlug);
  void requestExpiry(QString operationSlug);

 private:
  std::unordered_map<QString, QNetworkReply*> tagRequests;
  std::unordered_map<QString, TagCacheItem> cache;
};

#endif // TAGCACHE_H
