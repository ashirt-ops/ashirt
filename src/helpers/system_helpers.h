#pragma once

#include <QString>
#include <QDir>

#include "appconfig.h"
#include "appsettings.h"

class SystemHelpers {

 public:

  /// Returns (and creates, if necessary) the path to where evidence should be stored (includes
  /// ending path separator)
  static QString pathToEvidence() {
    AppConfig &conf = AppConfig::getInstance();
    auto op = AppSettings::getInstance().operationSlug();
    auto root = QStringLiteral("%1/").arg(conf.evidenceRepo);
    if (!op.isEmpty()) {
      root.append(QStringLiteral("%1/").arg(op));
    }

    QDir().mkpath(root);
    return root;
  }

};
