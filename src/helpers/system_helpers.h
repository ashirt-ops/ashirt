#pragma once

#include <QString>
#include <QDir>

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

};
