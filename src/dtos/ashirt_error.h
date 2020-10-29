#ifndef DTO_ASHIRT_ERROR_H
#define DTO_ASHIRT_ERROR_H

#include <QString>

#include "helpers/jsonhelpers.h"

namespace dto {
class AShirtError {

 public:
  QString error = "";

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
#endif // DTO_ASHIRT_ERROR_H
