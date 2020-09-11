#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <QStandardPaths>
#include <QString>

#include <iostream>

class Constants {
 public:
  static QString configLocation() {
    return QStandardPaths::writableLocation(QStandardPaths::DataLocation) + "/config.json";
  }

  static QString dbLocation() {
    return QStandardPaths::writableLocation(QStandardPaths::DataLocation) + "/evidence.sqlite";
  }

  static QString defaultEvidenceRepo() {
    return QStandardPaths::writableLocation(QStandardPaths::DataLocation) + "/evidence";
  }

};

#endif // CONSTANTS_H
