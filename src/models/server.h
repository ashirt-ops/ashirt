#ifndef MODEL_SERVER_H
#define MODEL_SERVER_H

#include <QString>
#include <QDateTime>
#include <QUuid>

namespace model {
/**
 * @brief The Server class represents a row in the servers table. This comprises:
 *        {uuid, name, accessKey, secretKey, hostPath, deletedAt}
 *        Unlike a typical data class, this also exports some extra functionality for easy use.
 */
class Server {

 public:
  Server(){}
  Server(QString uuid, QString name, QString accessKey, QString secretKey, QString hostPath) {
    this->serverUuid = uuid;
    this->serverName = name;
    this->accessKey = accessKey;
    this->secretKey = secretKey;
    this->hostPath = hostPath;
  }

 public:
  /// isEmpty is a small helper to determine if accessKey, secretKey, serverName, and hostPath are
  /// all empty strings. Returns true if this is the case, false otherwise
  bool isEmpty() {
    // in a funny order to align columns cleanly
    return accessKey.isEmpty() && serverName.isEmpty() &&
           secretKey.isEmpty() && hostPath.isEmpty();
  }
  /// isEmpty is a small helper to determine if accessKey, secretKey, serverName, and hostPath are
  /// all non-empty strings. Returns true if this is the case, false otherwise
  bool isComplete() {
    return !accessKey.isEmpty() && !serverName.isEmpty() &&
           !secretKey.isEmpty() && !hostPath.isEmpty();
  }

  bool isDeleted() {
    return deletedAt != nullptr;
  }

 public:
  static QString newUUID() { return formatUuid(QUuid::createUuid()); }
  static QString formatUuid(QUuid strUuid) {
    // trim first and last characters -- these are curly braces
    auto uuid = strUuid.toString().remove(0, 1);
    uuid.chop(1);
    return uuid;
  }

 public:
  QString serverName = "";
  QString serverUuid = "";
  QString accessKey = "";
  QString secretKey = "";
  QString hostPath = "";
  /// deletedAt stores the datetime when server was deleted, or nullptr otherwise
  QDateTime* deletedAt = nullptr;
};
}  // namespace model

#endif // MODEL_SERVER_H
