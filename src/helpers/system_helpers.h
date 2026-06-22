#pragma once

#include <QDir>
#include <QtGui/QGuiApplication>
#include <QtGui/QStyleHints>

#include "appconfig.h"
#include "string_helpers.h"

class SystemHelpers {

 public:

  /// Returns (and creates, if necessary) the path to where evidence should be stored (includes
  /// ending path separator)
  static QString pathToEvidence() {
    auto op = AppConfig::operationSlug();
    auto root = QStringLiteral("%1/").arg(AppConfig::value(CONFIG::EVIDENCEREPO));
    if (!op.isEmpty()) {
      if (!StringHelpers::isValidOperationSlug(op)) {
        qWarning() << "Invalid operation slug detected, ignoring:" << op;
      }
      else {
        root.append(QStringLiteral("%1/").arg(op));
      }
    }

    QDir().mkpath(root);
    return root;
  }
  static bool isLightTheme() {
    // QStyleHints::colorScheme() (Qt 6.5+) reports the platform theme directly
    // on Windows, macOS, and Linux. Treat an Unknown scheme as light.
    return QGuiApplication::styleHints()->colorScheme() != Qt::ColorScheme::Dark;
  }
};
