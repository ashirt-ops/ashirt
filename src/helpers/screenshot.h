// Copyright 2020, Verizon Media
// Licensed under the terms of MIT. See LICENSE file in project root for terms.

#ifndef SCREENSHOT_H
#define SCREENSHOT_H

#include <QObject>
#include <string>

class Screenshot : public QObject {
  Q_OBJECT
 public:
  Screenshot(QObject* parent = 0);
  void captureArea();
  void captureWindow();

 signals:
  void onScreenshotCaptured(QString filepath);

 private:
  QString formatScreenshotCmd(QString cmdProto, const QString& filename);
  void basicScreenshot(QString cmdProto);
};

#endif  // SCREENSHOT_H
