#pragma once

#include <QVariant>

#include "helpers/jsonhelpers.h"

namespace dto {
class Operation {
 public:
  Operation() {}
  Operation(QString name, QString slug) {
    this->name = name;
    this->slug = slug;
  }

  enum OperationStatus {
    OperationStatusPlanning = 0,
    OperationStatusAcitve = 1,
    OperationStatusComplete = 2,
  };

  QString slug;
  QString name;
  int numUsers;
  OperationStatus status;

  static Operation parseData(QByteArray data) {
    return parseJSONItem<Operation>(data, Operation::fromJson);
  }

  static QList<Operation> parseDataAsList(QByteArray data) {
    return parseJSONList<Operation>(data, Operation::fromJson);
  }

  static QByteArray createOperationJson(QString name, QString slug) {
    return createOperationJson(Operation(name, slug));
  }

  static QByteArray createOperationJson(Operation o) {
    QJsonObject obj;
    obj.insert(QStringLiteral("slug"), o.slug);
    obj.insert(QStringLiteral("name"), o.name);
    return QJsonDocument(obj).toJson();
  }

 private:
  // provides a Operation from a given QJsonObject
  static Operation fromJson(QJsonObject obj) {
    Operation o;
    o.slug = obj.value(QStringLiteral("slug")).toString();
    o.name = obj.value(QStringLiteral("name")).toString();
    o.numUsers = obj.value(QStringLiteral("numUsers")).toInt();
    o.status = static_cast<OperationStatus>(obj.value(QStringLiteral("status")).toInt());
    return o;
  }
};
}  // namespace dto
