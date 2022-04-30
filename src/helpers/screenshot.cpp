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
#include "helpers/system_helpers.h"

Screenshot::Screenshot(QObject *parent) : QObject(parent) {}

QString Screenshot::formatScreenshotCmd(QString cmdProto, const QString &filename) {
  auto lowerCmd = cmdProto.toLower();
  QString key = QStringLiteral("%file");
  auto idx = lowerCmd.indexOf(key);
  if (idx == -1) {
    return cmdProto;
  }
  QString fixedFilename = QStringLiteral("'%1'").arg(filename);
  return cmdProto.replace(idx, key.length(), fixedFilename);
}

void Screenshot::captureArea() { basicScreenshot(AppConfig::getInstance().screenshotExec); }

void Screenshot::captureWindow() { basicScreenshot(AppConfig::getInstance().captureWindowExec); }

QString Screenshot::mkName() {
  return FileHelpers::randomFilename(QStringLiteral("ashirt_screenshot_XXXXXX.%1").arg(extension()));
}
QString Screenshot::contentType() { return QStringLiteral("image"); }
QString Screenshot::extension() { return QStringLiteral("png"); }


void Screenshot::basicScreenshot(QString cmdProto) {
  auto root = SystemHelpers::pathToEvidence();
  auto hasPath = QDir().mkpath(root);

  if (hasPath) {
    auto tempPath = QStringLiteral("%1/%2").arg(QDir::tempPath(), mkName());

    QString cmd = formatScreenshotCmd(std::move(cmdProto), tempPath);
    auto lastSlash = tempPath.lastIndexOf(QStringLiteral("/")) + 1;
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
