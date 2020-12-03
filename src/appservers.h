#ifndef APPSERVERS_H
#define APPSERVERS_H

#include <QObject>

#include "config/server_item.h"
#include "config/server_set.h"

class AppServers : public QObject {
  Q_OBJECT;

 public:
  static AppServers &getInstance() {
    static AppServers instance;
    return instance;
  }
  AppServers(AppServers const &) = delete;
  void operator=(AppServers const &) = delete;

 private:
  AppServers() {
    loadServers();
  }

 private:
  QString getServersFileLocation(const QString& path);

  ServerItem selectServer(QString serverUuid="");
  QString selectServerUuid(QString maybeServerUuid);

 public:
  bool isLoadSuccessful();
  void loadServers(const QString& path="");
  void writeServers(const QString& path="");
  void addServer(const ServerItem& item);
  void deleteServer(const QString& serverUuid);
  void purgeServer(const QString& serverUuid);
  void restoreServer(const QString& serverUuid);
  ServerItem getServerByUuid(const QString& serverUuid);
  std::vector<ServerItem> getServers(bool includeDeleted=false);
  void updateServer(ServerItem item);

  QString currentServerUuid();
  QString accessKey(QString serverUuid="");
  QString secretKey(QString serverUuid="");
  QString hostPath(QString serverUuid="");
  QString serverName(QString serverUuid="");

 signals:
  void listUpdated();


 private:
  ServerSet* serverSet;
};

#endif // APPSERVERS_H
