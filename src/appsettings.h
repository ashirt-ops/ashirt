// Copyright 2020, Verizon Media
// Licensed under the terms of MIT. See LICENSE file in project root for terms.

#ifndef APPSETTINGS_H
#define APPSETTINGS_H

#include <QSequentialIterable>
#include <QSettings>
#include <QString>
#include <QMap>

#include "helpers/constants.h"
#include "models/tag.h"
#include "models/server_setting.h"

// AppSettings is a singleton construct for accessing the application's settings. This is different
// from configuration, as it represents the application's state, rather than how the application
// communicates.
//
// singleton design borrowed from:
// https://stackoverflow.com/questions/1008019/c-singleton-design-pattern
class AppSettings : public QObject {
  Q_OBJECT;

 public:
  static AppSettings &getInstance() {
    static AppSettings instance;
    return instance;
  }
  AppSettings(AppSettings const &) = delete;
  void operator=(AppSettings const &) = delete;

 private:
  AppSettings() : QObject(nullptr) {}

 public:
 signals:
  void onOperationUpdated(QString operationSlug, QString operationName);
  void onServerUpdated(QString serverUuid);
  void onSettingsSynced();

 public:
  void sync() {
    settings.sync();  // ignoring the error
    emit this->onSettingsSynced();
  }

  void upgrade() {
    int version = getVersion();
    if (version < 2) {
      upgradeToV2();
    }
  }

 private:
  // internal fields
  unsigned int getVersion() { return settings.value(settingVersion).toUInt(); }
  void setVersion(unsigned int versionNumber) { settings.setValue(settingVersion, versionNumber); }

 private:
  void upgradeToV2() {
    auto defaultServerUuid = Constants::legacyServerUuid();
    setServerUuid(defaultServerUuid);

    auto currentOpSlug = settings.value(opSlugSetting).toString();
    auto currentOpName = settings.value(opNameSetting).toString();

    if (currentOpName != "" && currentOpSlug != "") {
      updateServerSetting(defaultServerUuid, model::ServerSetting(currentOpName, currentOpSlug));
    }
    settings.remove(opSlugSetting);
    settings.remove(opNameSetting);
  }

 public:
  void setOperationDetails(QString opSlug, QString opName) {
    auto setting = getActiveServerSettings();
    setting.activeOperationName = opName;
    setting.activeOperationSlug = opSlug;
    updateActiveServerSetting(setting);

    emit onOperationUpdated(opSlug, opName);
  }
  QString operationSlug() { return getActiveServerSettings().activeOperationSlug; }
  QString operationName() { return getActiveServerSettings().activeOperationName; }

  void setLastUsedTags(std::vector<model::Tag> lastTags) {
    QVariantList writeTags;
    
    for (auto tag : lastTags) {
      writeTags << QVariant::fromValue(tag);
    }

    settings.setValue(lastUsedTagsSetting, QVariant::fromValue(writeTags));
  }

  std::vector<model::Tag> getLastUsedTags() {
    std::vector<model::Tag> rtn;

    auto val = settings.value(lastUsedTagsSetting);

    if (val.canConvert<QVariantList>()) {
      QSequentialIterable iter = val.value<QSequentialIterable>();
      for (const QVariant& item : iter) {
        rtn.push_back(qvariant_cast<model::Tag>(item));
      }
    }

    return rtn;
  }

  QMap<QString, model::ServerSetting> getKnownServers() {
    auto val = settings.value(knownServersSetting);
    return qvariant_cast<QMap<QString, model::ServerSetting>>(val);
  }
  void setKnownServers(QMap<QString, model::ServerSetting> servers) {
    settings.setValue(knownServersSetting, QVariant::fromValue(servers));
  }
  void updateServerSetting(QString serverUuid, model::ServerSetting newSetting, QString oldServerUuid="") {
    auto servers = getKnownServers();
    if (oldServerUuid != "") {
      removeServerSetting(oldServerUuid);
    }
    servers[serverUuid] = newSetting;
    setKnownServers(servers);
  }
  void updateActiveServerSetting(model::ServerSetting newSetting) {
    updateServerSetting(serverUuid(), newSetting);
  }
  model::ServerSetting getActiveServerSettings() {
    auto servers = getKnownServers();
    auto itr = servers.find(serverUuid());
    if (itr != servers.end()) {
      return itr.value();
    }
    return model::ServerSetting();
  }
  bool removeServerSetting(QString serverUuid) {
    auto servers = getKnownServers();
    return servers.remove(serverUuid) != 0;
  }

  void setServerUuid(QString updatedServerUuid) {
    settings.setValue(activeServerSetting, updatedServerUuid);
    auto activeServerSetting = getActiveServerSettings();
    emit onOperationUpdated(activeServerSetting.activeOperationSlug, activeServerSetting.activeOperationName);
    emit onServerUpdated(updatedServerUuid);
  }
  QString serverUuid() { return settings.value(activeServerSetting).toString(); }

 private:
  QSettings settings;

  // deprecated settings
  const char *opSlugSetting = "operation/slug"; // removed in v2 -- moved into server/known
  const char *opNameSetting = "operation/name"; // removed in v2 -- moved into server/known

  // active settings
  const char *settingVersion  = "settings/version";
  const char *activeServerSetting = "server/active";
  const char *knownServersSetting = "server/known";
  const char *lastUsedTagsSetting = "gather/tags";
};
#endif  // APPSETTINGS_H
