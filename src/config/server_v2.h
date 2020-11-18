#ifndef SERVER_V2_H
#define SERVER_V2_H

#include <QJsonArray>
#include <unordered_map>
#include <vector>

#include "server_set.h"
#include "helpers/jsonhelpers.h"
#include "helpers/file_helpers.h"
#include "config/server_item.h"

class ServersV2 : public ServerSet {
 public:
  ServersV2() = default;

 public:
  static QJsonArray serialize(ServersV2 manifest) {
    QJsonArray a;
    std::vector<ServerItem> serverList = manifest.getServers(true);
    for(auto item : serverList) {
      a.push_back(ServerItem::serialize(item));
    }
    return a;
  }

 public:
  void deserialize(QByteArray rawData, bool clearServers=true) {
    auto servers = parseJSONList<ServerItem>(
        rawData, &ServerItem::deserialize);
    if (clearServers) {
      entries.clear();
    }
    for (ServerItem s : servers) {
      entries[s.getServerUuid()] = s;
    }
  }

  ServerItem getServerByUuid(const QString& uuid) override {
    auto position = entries.find(uuid);
    if (position == entries.end()) {
      return ServerItem();
    }
    return position->second;
  }

  std::vector<ServerItem> getServers(bool includeDeleted) override {
    // this first part re-orders the servers so they are always returned in a consistent order
    std::vector<ServerItem> serverList;
    serverList.reserve(entries.size());
    for (auto serverEntry : entries) {
      serverList.push_back(serverEntry.second);
    }
    std::sort(serverList.begin(), serverList.end(), [](ServerItem a, ServerItem b){return a.getId() < b.getId();});

    if (!includeDeleted) {
      serverList.erase( // resize to the new length
          std::remove_if(serverList.begin(), serverList.end(), [](ServerItem item){ return item.deleted;}), // remove deleted elements
          serverList.end());
    }

    return serverList;
  }

  ServerItem deleteServer(const QString& uuid, bool undelete) override {
    auto position = entries.find(uuid);
    if (position == entries.end()) {
      return ServerItem();
    }
    position->second.deleted = undelete ? false : true;
    updateServersFile();
    return position->second;
  }

  void addServer(ServerItem item) override {
    if (item.isValid()) {
      auto itemCopy = ServerItem(
          entries.size() + 1, item.getServerUuid(),
          item.serverName, item.accessKey, item.secretKey, item.hostPath, item.deleted);

      entries[itemCopy.getServerUuid()] = itemCopy;
      updateServersFile();
    }
  }

  void updateServer(ServerItem item) override {
    auto foundItem = entries.find(item.getServerUuid());
    if (foundItem != entries.end()) {
      // TODO: will this work, or do we need to reinsert into the entries?
      // only moving fields that are editable -- other fields are ignored.
      foundItem->second.serverName = item.serverName;
      foundItem->second.accessKey = item.accessKey;
      foundItem->second.secretKey = item.secretKey;
      foundItem->second.hostPath = item.hostPath;
    }
  }

 protected:
  void parseConfigData(const QByteArray& data) override {
    deserialize(data);
  }

  QByteArray toFileEncoding() override {
    auto data = serialize(*this);
    QJsonDocument doc(data);
    return doc.toJson();
  }

  QString getLoadedPath() override {
    return loadedPath;
  }
  void setLoadedPath(QString path) override {
    this->loadedPath = path;
  }

  void makeDefaultServerset() override {

  }

 private:
  void updateServersFile() {
    writeToFile(getLoadedPath());
  }

 private:
  std::unordered_map<QString, ServerItem> entries;
  QString loadedPath;
};

#endif // SERVER_V2_H
