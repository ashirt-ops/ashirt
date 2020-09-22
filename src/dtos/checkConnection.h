#ifndef TESTCONNECTION_H
#define TESTCONNECTION_H

#include <QVariant>

#include "helpers/jsonhelpers.h"

namespace dto {
class CheckConnection {
 public:
  CheckConnection(){}

  bool ok;
  bool parsedCorrectly;

  static CheckConnection parseJson(QByteArray data) {
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);
    CheckConnection cc;

    cc.parsedCorrectly = false;
    if (err.error == QJsonParseError::NoError) {
      QJsonValue val = doc["ok"];
      if (!val.isUndefined()) {
        cc.parsedCorrectly = true;
        cc.ok = val.toBool();
      }
    }

    return cc;
  }
};
}

#endif // TESTCONNECTION_H
