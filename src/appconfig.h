// Copyright 2020, Verizon Media
// Licensed under the terms of MIT. See LICENSE file in project root for terms.

#ifndef DATA_H
#define DATA_H

#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>
#include <QStringLiteral>
#include <cstdlib>
#include <stdexcept>

#include "exceptions/fileerror.h"

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

  QString evidenceRepo = "";
  QString accessKey = "";
  QString secretKey = "";
  QString apiURL = "";
  QString screenshotExec = "";
  QString screenshotShortcutCombo = "";
  QString captureWindowExec = "";
  QString captureWindowShortcut = "";
  QString captureCodeblockShortcut = "";

  QString errorText = "";

 private:
  AppConfig() noexcept {
    try {
      readConfig();
    }
    catch (std::exception &e) {
      errorText = e.what();
    }
  }

  QString saveLocation =
      QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/ashirt/config.json";

  void readConfig() {
    QFile configFile(saveLocation);
    if (!configFile.open(QIODevice::ReadOnly)) {
      if (configFile.exists()) {
        throw FileError::mkError("Error reading config file", saveLocation.toStdString(),
                                 configFile.error());
      }
      try {
        writeDefaultConfig();
      }
      catch (...) {
        // ignoring -- just trying to generate an empty config
      }
      return;
    }

    QByteArray data = configFile.readAll();
    if (configFile.error() != QFile::NoError) {
      throw FileError::mkError("Error reading config file", saveLocation.toStdString(),
                               configFile.error());
    }

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);
    if (err.error != QJsonParseError::NoError) {
      // ignoring specific type -- unlikely to occur in practice.
      throw std::runtime_error("Unable to parse config file");
    }

    this->evidenceRepo = doc["evidenceRepo"].toString();
    this->accessKey = doc["accessKey"].toString();
    this->secretKey = doc["secretKey"].toString();
    this->apiURL = doc["apiURL"].toString();
    this->screenshotExec = doc["screenshotCommand"].toString();
    this->screenshotShortcutCombo = doc["screenshotShortcut"].toString();
    this->captureWindowExec = doc["captureWindowExec"].toString();
    this->captureWindowShortcut = doc["captureWindowShortcut"].toString();
    this->captureCodeblockShortcut = doc["captureCodeblockShortcut"].toString();
  }

  void writeDefaultConfig() {
    evidenceRepo = QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/ashirt";

#ifdef Q_OS_MACOS
    screenshotExec = "screencapture -s %file";
    captureWindowExec = "screencapture -w %file";
#endif

    try {
      writeConfig();
    }
    catch (...) {
      // ignoring error -- best effort approach
    }
  }

 public:
  void writeConfig() {
    QJsonObject root = QJsonObject();  // QFiles close automatically, so no need for close here.
    root["evidenceRepo"] = evidenceRepo;
    root["accessKey"] = accessKey;
    root["secretKey"] = secretKey;
    root["apiURL"] = apiURL;
    root["screenshotCommand"] = screenshotExec;
    root["screenshotShortcut"] = screenshotShortcutCombo;
    root["captureWindowExec"] = captureWindowExec;
    root["captureWindowShortcut"] = captureWindowShortcut;
    root["captureCodeblockShortcut"] = captureCodeblockShortcut;

    auto saveRoot = saveLocation.left(saveLocation.lastIndexOf("/"));
    QDir().mkpath(saveRoot);
    QFile configFile(saveLocation);
    if (!configFile.open(QIODevice::WriteOnly)) {
      throw FileError::mkError("Error writing config file", saveLocation.toStdString(),
                               configFile.error());
    }

    QJsonDocument doc(root);
    auto written = configFile.write(doc.toJson());
    if (written == -1) {
      throw FileError::mkError("Error writing config file", saveLocation.toStdString(),
                               configFile.error());
    }

    return;
  }
};

#endif  // DATA_H
