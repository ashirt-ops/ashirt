#pragma once

#include <QStandardPaths>

class Constants {
 public:
  inline static const auto dbLocation = QStringLiteral("%1/evidence.sqlite").arg(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation));
  inline static const auto defaultEvidenceRepo = QStringLiteral("%1/evidence").arg(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation));
  /// defaultDbName returns a string storing the "name" of the database for Qt identification
  /// purposes. This _value_ should not be reused for other db connections.
  inline static const auto defaultDbName = QStringLiteral("evidence");
#ifdef Q_OS_MACOS
  inline static const auto codeFont = QStringLiteral("monaco");
#else
  inline static const auto codeFont = QStringLiteral("source code pro");
#endif
};
