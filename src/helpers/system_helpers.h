#pragma once

#include <QDir>
#include <QtGui/QGuiApplication>
#include <QtGui/QStyleHints>

#include "appconfig.h"
#include "helpers/file_helpers.h"

class SystemHelpers {

 public:

  /// Returns (and creates, if necessary) the path to where evidence should be stored (includes
  /// ending path separator)
  static QString pathToEvidence() {
    auto op = AppConfig::operationSlug();
    auto root = QStringLiteral("%1/").arg(AppConfig::value(CONFIG::EVIDENCEREPO));
    if (!op.isEmpty()) {
      auto opRoot = QStringLiteral("%1%2/").arg(root, op);
      if (FileHelpers::isWithin(root, opRoot))
        root = opRoot;
      else
        qWarning() << "Operation slug resolves outside evidence directory, ignoring:" << op;
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
