#pragma once

#include "helpers/jsonhelpers.h"

namespace dto {
class AShirtError {

 public:
  QString error;

  static AShirtError parseData(QByteArray data) {
    return parseJSONItem<AShirtError>(data, AShirtError::fromJson);
  }

 private:
  static AShirtError fromJson(QJsonObject obj) {
    AShirtError e;
    e.error = obj["error"].toString();

    return e;
  }
};
}
