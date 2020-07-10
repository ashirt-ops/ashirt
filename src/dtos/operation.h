// Copyright 2020, Verizon Media
// Licensed under the terms of MIT. See LICENSE file in project root for terms.

#ifndef DTO_OPERATION_H
#define DTO_OPERATION_H

#include <QVariant>
#include <vector>

#include "helpers/jsonhelpers.h"

namespace dto {
class Operation {
 public:
  Operation() {}

  enum OperationStatus {
    OperationStatusPlanning = 0,
    OperationStatusAcitve = 1,
    OperationStatusComplete = 2,
  };

  QString slug;
  QString name;
  int numUsers;
  OperationStatus status;
  qint64 id;

  static Operation parseData(QByteArray data) {
    return parseJSONItem<Operation>(data, Operation::fromJson);
  }

  static std::vector<Operation> parseDataAsList(QByteArray data) {
    return parseJSONList<Operation>(data, Operation::fromJson);
  }

 private:
  // provides a Operation from a given QJsonObject
  static Operation fromJson(QJsonObject obj) {
    Operation o;
    o.slug = obj["slug"].toString();
    o.name = obj["name"].toString();
    o.numUsers = obj["numUsers"].toInt();
    o.status = static_cast<OperationStatus>(obj["status"].toInt());
    o.id = obj["id"].toVariant().toLongLong();

    return o;
  }
};
}  // namespace dto

#endif  // DTO_OPERATION_H
