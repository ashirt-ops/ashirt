#ifndef TESTCONNECTION_H
#define TESTCONNECTION_H

#include <QVariant>

#include "helpers/jsonhelpers.h"

namespace dto {
class CheckConnection {
 public:
  CheckConnection(){}

  bool ashirtConnected;
  bool parsedCorrectly;

  static CheckConnection parseJson(QByteArray data) {
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);
    CheckConnection cc;

    cc.parsedCorrectly = false;
    if (err.error == QJsonParseError::NoError) {
      QJsonValue val = doc["ashirtConnected"];
      if (!val.isUndefined()) {
        cc.parsedCorrectly = true;
        cc.ashirtConnected = val.toBool();
      }
    }

    return cc;
  }
};
}

#endif // TESTCONNECTION_H
