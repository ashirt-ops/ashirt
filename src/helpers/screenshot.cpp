// Copyright 2020, Verizon Media
// Licensed under the terms of MIT. See LICENSE file in project root for terms.

#include "screenshot.h"

#include <QDir>
#include <QFile>
#include <QObject>

#include <array>
#include <cstdio>
#include <iostream>
#include <string>
#include <utility>

#include "appconfig.h"
#include "helpers/file_helpers.h"

Screenshot::Screenshot(QObject *parent) : QObject(parent) {}

QString Screenshot::formatScreenshotCmd(QString cmdProto, const QString &filename) {
  auto lowerCmd = cmdProto.toLower();
  QString key = "%file";
  auto idx = lowerCmd.indexOf(key);
  if (idx == -1) {
    return cmdProto;
  }
  QString fixedFilename = "'" + filename + "'";
  return cmdProto.replace(idx, key.length(), fixedFilename);
}

void Screenshot::captureArea() { basicScreenshot(AppConfig::getInstance().screenshotExec); }

void Screenshot::captureWindow() { basicScreenshot(AppConfig::getInstance().captureWindowExec); }

void Screenshot::basicScreenshot(QString cmdProto) {
  auto root = FileHelpers::pathToEvidence();
  auto hasPath = QDir().mkpath(root);

  if (hasPath) {
    auto tempPath = FileHelpers::randomFilename(QDir::tempPath() + "/ashirt_screenshot_XXXXXX.png");

    QString cmd = formatScreenshotCmd(std::move(cmdProto), tempPath);
    auto lastSlash = tempPath.lastIndexOf("/") + 1;
    QString tempName = tempPath.right(tempPath.length() - lastSlash);

    system(cmd.toStdString().c_str());

    // check if file exists before doing this
    auto finalName = root + tempName;
    QFile src(tempPath);
    if (src.exists()) {
      auto moved = src.rename(QString(finalName));
      auto trueName = moved ? finalName : tempName;
      emit onScreenshotCaptured(trueName);
    }
  }
}
