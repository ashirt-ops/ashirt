#pragma once

#include <QDir>

#ifdef Q_OS_WIN
#include <QSettings>
#else
#include <QtGui/QGuiApplication>
#include <QtGui/QPalette>
#endif

#include "appconfig.h"

class SystemHelpers {

 public:

  /// Returns (and creates, if necessary) the path to where evidence should be stored (includes
  /// ending path separator)
  static QString pathToEvidence() {
    auto op = AppConfig::operationSlug();
    auto root = QStringLiteral("%1/").arg(AppConfig::value(CONFIG::EVIDENCEREPO));
    if (!op.isEmpty()) {
      root.append(QStringLiteral("%1/").arg(op));
    }

    QDir().mkpath(root);
    return root;
  }
  static bool isLightTheme() {
#ifdef Q_OS_WIN
    QSettings settings(QStringLiteral("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize"), QSettings::NativeFormat);
    return settings.value(QStringLiteral("SystemUsesLightTheme")).toInt() == 1;
#else
    return qApp->palette().text().color().value() <= QColor(Qt::lightGray).value();
#endif
  }
};
