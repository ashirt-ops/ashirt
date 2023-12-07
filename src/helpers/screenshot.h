#pragma once

#include <QObject>

class Screenshot : public QObject {
  Q_OBJECT
 public:
  Screenshot(QObject* parent = nullptr);
  void captureArea();
  void captureWindow();

  static QString mkName();
  static QString extension() { return QStringLiteral("png"); }
  static QString contentType() { return QStringLiteral("image"); }

 signals:
  void onScreenshotCaptured(QString filepath);

 private:
  void basicScreenshot(QString cmdProto);
  inline static const QString m_fileTemplate = QStringLiteral("%1/%2");
  inline static const QString m_doubleQuote = QStringLiteral("\"");
  inline static const QString m_space = QStringLiteral(" ");
};
