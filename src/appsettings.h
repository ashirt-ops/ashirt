// Copyright 2020, Verizon Media
// Licensed under the terms of MIT. See LICENSE file in project root for terms.

#ifndef APPSETTINGS_H
#define APPSETTINGS_H

#include <QSettings>
#include <QString>

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
  QSettings settings;

  const char *opSlugSetting = "operation/slug";
  const char *opNameSetting = "operation/name";

  AppSettings() : QObject(nullptr) {}

 public:
 signals:
  void onOperationUpdated(QString operationSlug, QString operationName);
  void onSettingsSynced();

 public:
  void sync() {
    settings.sync();  // ignoring the error
    emit this->onSettingsSynced();
  }

  void setOperationDetails(QString operationSlug, QString operationName) {
    settings.setValue(opSlugSetting, operationSlug);
    settings.setValue(opNameSetting, operationName);

    emit onOperationUpdated(operationSlug, operationName);
  }
  QString operationSlug() { return settings.value(opSlugSetting).toString(); }
  QString operationName() { return settings.value(opNameSetting).toString(); }
};
#endif  // APPSETTINGS_H
