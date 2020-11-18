#include "appservers.h"

#include "helpers/file_helpers.h"
#include "helpers/constants.h"
#include "config/server_v2.h"

QString AppServers::getServersFileLocation(const QString& path) {
  return path.isEmpty() ? Constants::serversLocation() : path;
}

void AppServers::loadServers(const QString& path) {
  auto filepath = getServersFileLocation(path);

  // eventually, figure out how to load the proper version

  serverSet = new ServersV2();
  serverSet->readFromServers(filepath);
}

bool AppServers::isLoadSuccessful() {
  return serverSet->isValid();
}

void AppServers::writeServers(const QString& path) {
  serverSet->writeToFile(getServersFileLocation(path));
}

void AppServers::addServer(const ServerItem& item) {
  serverSet->addServer(item);
  serverSet->writeToFile(getServersFileLocation(""));
}

void AppServers::deleteServer(const QString& serverUuid) {
  serverSet->deleteServer(serverUuid);
  serverSet->writeToFile(getServersFileLocation(""));
}

void AppServers::restoreServer(const QString& serverUuid) {
  serverSet->deleteServer(serverUuid, true);
  serverSet->writeToFile(getServersFileLocation(""));
}

ServerItem AppServers::getServerByUuid(const QString& serverUuid) {
  return serverSet->getServerByUuid(serverUuid);
}

std::vector<ServerItem> AppServers::getServers(bool includeDeleted) {
  return serverSet->getServers(includeDeleted);
}

void AppServers::updateServer(ServerItem item) {
  serverSet->updateServer(item);
}

ServerItem AppServers::selectServer(QString serverUuid) {
  return getServerByUuid(selectServerUuid(serverUuid));
}

QString AppServers::currentServerUuid() {
  return AppSettings::getInstance().serverUuid();
}

QString AppServers::selectServerUuid(QString maybeServerUuid) {
  return maybeServerUuid = (maybeServerUuid == "") ? currentServerUuid() : maybeServerUuid;
}

QString AppServers::accessKey(QString serverUuid) {
  return selectServer(serverUuid).accessKey;
}
QString AppServers::secretKey(QString serverUuid) {
  return selectServer(serverUuid).secretKey;
}
QString AppServers::hostPath(QString serverUuid) {
  return selectServer(serverUuid).hostPath;
}
QString AppServers::serverName(QString serverUuid) {
  return selectServer(serverUuid).serverName;
}
