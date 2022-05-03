#pragma once

#include <QObject>
#include <QMap>

#include "tagcacheitem.h"
#include "dtos/tag.h"

class QNetworkReply;

class TagCache : public QObject {
  Q_OBJECT
 public:
  TagCache(QObject *parent =nullptr);
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
  QMap<QString , QNetworkReply*> tagRequests;
  QMap<QString, TagCacheItem> cache;
};
