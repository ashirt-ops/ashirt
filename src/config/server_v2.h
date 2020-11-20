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
    dirty = true;
  }

  ServerItem getServerByUuid(const QString& uuid) override {
    auto position = entries.find(uuid);
    if (position == entries.end()) {
      return ServerItem();
    }
    return position->second;
  }

  std::vector<ServerItem> getServers(bool includeDeleted) override {
    static std::vector<ServerItem> fullServerList; // cache this list to avoid re-sorting a no-change list

    if (dirty) {
      std::cout << "re-generating server list" << std::endl;
      fullServerList.clear();
      fullServerList.reserve(entries.size());
      for (auto serverEntry : entries) {
        fullServerList.push_back(serverEntry.second);
      }
      std::sort(fullServerList.begin(), fullServerList.end(), [](ServerItem a, ServerItem b){return a.getId() < b.getId();});

      dirty = false; // remove the dirty flag, since the item has now been re-sorted
    }

    if (!includeDeleted) {
      std::vector<ServerItem> serverList;
      serverList.reserve(fullServerList.size());
      std::copy_if(fullServerList.begin(), fullServerList.end(), std::back_inserter(serverList),
                   [](ServerItem item){return !item.deleted;});
      return serverList;
    }

    return fullServerList;
  }

  ServerItem deleteServer(const QString& uuid, bool undelete) override {
    auto position = entries.find(uuid);
    if (position == entries.end()) {
      return ServerItem();
    }
    position->second.deleted = undelete ? false : true;
    updateServersFile();
    dirty = true;
    return position->second;
  }

  void addServer(ServerItem item) override {
    if (item.isValid()) {
      auto knownServers = getServers(true);
      int newId = (knownServers.size() > 0) ? knownServers.back().getId() : 1;

      auto itemCopy = ServerItem(newId, item.getServerUuid(),
          item.serverName, item.accessKey, item.secretKey, item.hostPath, item.deleted);

      entries[itemCopy.getServerUuid()] = itemCopy;
      dirty = true;
      updateServersFile();
    }
  }

  void updateServer(ServerItem item) override {
    auto foundItem = entries.find(item.getServerUuid());
    if (foundItem != entries.end()) {
      // only moving fields that are editable -- other fields are ignored.
      foundItem->second.serverName = item.serverName;
      foundItem->second.accessKey = item.accessKey;
      foundItem->second.secretKey = item.secretKey;
      foundItem->second.hostPath = item.hostPath;

      dirty = true;
      updateServersFile();
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

 private:
  void updateServersFile() {
    writeToFile(getLoadedPath());
  }

 private:
  std::unordered_map<QString, ServerItem> entries;
  QString loadedPath;
  bool dirty = true;
};

#endif // SERVER_V2_H
