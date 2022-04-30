// Copyright 2020, Verizon Media
// Licensed under the terms of MIT. See LICENSE file in project root for terms.

#pragma once

#include <QObject>
#include <string>

class Screenshot : public QObject {
  Q_OBJECT
 public:
  Screenshot(QObject* parent = 0);
  void captureArea();
  void captureWindow();

  static QString mkName();
  static QString extension();
  static QString contentType();

 signals:
  void onScreenshotCaptured(QString filepath);

 private:
  QString formatScreenshotCmd(QString cmdProto, const QString& filename);
  void basicScreenshot(QString cmdProto);
};
