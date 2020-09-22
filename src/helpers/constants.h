#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <QStandardPaths>
#include <QString>

#include <iostream>

class Constants {
 public:
  static QString configLocation() {
#ifdef Q_OS_MACOS
    return QStandardPaths::writableLocation(QStandardPaths::DataLocation) + "/config.json";
#else
    return QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/ashirt/config.json";
#endif
  }

  static QString dbLocation() {
    return QStandardPaths::writableLocation(QStandardPaths::DataLocation) + "/evidence.sqlite";
  }

  static QString defaultEvidenceRepo() {
    return QStandardPaths::writableLocation(QStandardPaths::DataLocation) + "/evidence";
  }

};

#endif // CONSTANTS_H
