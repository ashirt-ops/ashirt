#pragma once

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
      QJsonValue val = doc.object().value(QStringLiteral("ok"));
      if (!val.isUndefined()) {
        cc.parsedCorrectly = true;
        cc.ok = val.toBool();
      }
    }

    return cc;
  }
};
}
