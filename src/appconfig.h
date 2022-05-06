// Copyright 2020, Verizon Media
// Licensed under the terms of MIT. See LICENSE file in project root for terms.

#pragma once

#include <QDir>
#include <QFile>
#include <QJsonValue>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>
#include <QStringLiteral>
#include <cstdlib>
#include <stdexcept>

#include "exceptions/fileerror.h"
#include "helpers/constants.h"
#include "helpers/file_helpers.h"
#include "helpers/jsonhelpers.h"

// AppConfig is a singleton construct for accessing the application's configuration.
// singleton design borrowed from:
// https://stackoverflow.com/questions/1008019/c-singleton-design-pattern
class AppConfig {
 public:
  static AppConfig &getInstance() {
    static AppConfig instance;
    return instance;
  }
  AppConfig(AppConfig const &) = delete;
  void operator=(AppConfig const &) = delete;

  QString evidenceRepo;
  QString accessKey;
  QString secretKey;
  QString apiURL;
  QString screenshotExec;
  QString screenshotShortcutCombo;
  QString captureWindowExec;
  QString captureWindowShortcut;
  QString captureCodeblockShortcut;

  QString errorText;

 private:
  AppConfig() noexcept {
    try {
      readConfig();
    }
    catch (std::exception &e) {
      errorText = e.what();
    }
  }

  /// readConfig attempts to read the provided path and parse the configuration file.
  /// If successful, the config file is loaded. If the config file is missing, then a
  /// default file will be generated. If some other error occurs, a FileError is thrown.
  void readConfig(QString location = Constants::configLocation) {
    QFile configFile(location);
    if (!configFile.open(QIODevice::ReadOnly)) {
      if (configFile.exists()) {
        throw FileError::mkError("Error reading config file", location.toStdString(),
                                 configFile.error());
      }
      writeDefaultConfig();
      return;
    }

    QByteArray data = configFile.readAll();
    if (configFile.error() != QFile::NoError) {
      throw FileError::mkError("Error reading config file", location.toStdString(),
                               configFile.error());
    }

    auto result = parseJSONItem<QString>(data, [this](QJsonObject src) {
      applyConfig(src);
      return "";
    });
    if (result.isNull()) {
      throw std::runtime_error("Unable to parse config file");
    }
  }

  /// writeDefaultConfig attempts to write a basic configuration to disk.
  /// This is useful on first runs/when no config data is set.
  void writeDefaultConfig() {
    evidenceRepo = Constants::defaultEvidenceRepo;

#ifdef Q_OS_MACOS
    screenshotExec = QStringLiteral("screencapture -s %file");
    captureWindowExec = QStringLiteral("screencapture -w %file");
#endif

    writeConfig();
  }

 public:

  /// applyConfig takes a parsed json configuration, and applies it to the current running app instance
  void applyConfig(QJsonObject src) {
    QList<QPair<QString, QString*>> fields = {
        QPair<QString, QString*>(QStringLiteral("evidenceRepo"), &evidenceRepo),
        QPair<QString, QString*>(QStringLiteral("accessKey"), &accessKey),
        QPair<QString, QString*>(QStringLiteral("secretKey"), &secretKey),
        QPair<QString, QString*>(QStringLiteral("apiURL"), &apiURL),
        QPair<QString, QString*>(QStringLiteral("screenshotCommand"), &screenshotExec),
        QPair<QString, QString*>(QStringLiteral("screenshotShortcut"), &screenshotShortcutCombo),
        QPair<QString, QString*>(QStringLiteral("captureWindowExec"), &captureWindowExec),
        QPair<QString, QString*>(QStringLiteral("captureWindowShortcut"), &captureWindowShortcut),
        QPair<QString, QString*>(QStringLiteral("captureCodeblockShortcut"), &captureCodeblockShortcut),
    };

    for (auto fieldPair : fields) {
      QJsonValue val = src.value(fieldPair.first);
      if (!val.isUndefined() && val.isString()) {
        *fieldPair.second = val.toString();
      }
    }
  }

  /// serializeConfig creates a Json Object from the currently-used configuration
  QJsonObject serializeConfig() {
    QJsonObject root;
    root["evidenceRepo"] = evidenceRepo;
    root["accessKey"] = accessKey;
    root["secretKey"] = secretKey;
    root["apiURL"] = apiURL;
    root["screenshotCommand"] = screenshotExec;
    root["screenshotShortcut"] = screenshotShortcutCombo;
    root["captureWindowExec"] = captureWindowExec;
    root["captureWindowShortcut"] = captureWindowShortcut;
    root["captureCodeblockShortcut"] = captureCodeblockShortcut;
    return root;
  }

  /// writeConfig serializes the running config, and writes the assoicated file to the given path.
  /// The path defaults to Constants::configLocation()
  void writeConfig(QString alternateSavePath = QString()) {
    QString writeLoc = alternateSavePath.isEmpty() ? Constants::configLocation : alternateSavePath;

    auto configContent = QJsonDocument(serializeConfig()).toJson();

    try {
      FileHelpers::mkdirs(writeLoc, true); // ensure the path exists
      FileHelpers::writeFile(writeLoc, configContent);
    }
    catch(const FileError &e) {
      // rewrap error for easier identification
      throw FileError::mkError("Error writing config file", writeLoc.toStdString(),
                               e.fileDeviceError);
    }
    return;
   }
};
