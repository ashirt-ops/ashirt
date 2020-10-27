#ifndef SERVER_SETTING_H
#define SERVER_SETTING_H

#include <QString>
#include <QVariant>

namespace model {
class ServerSetting {
 public:
  ServerSetting() = default;
  ServerSetting(QString name, QString slug) {
    this->activeOperationName = name;
    this->activeOperationSlug = slug;
  }

 public:
  QString activeOperationSlug = "";
  QString activeOperationName = "";
};
}  // namespace model

Q_DECLARE_METATYPE(model::ServerSetting);

#endif // SERVER_SETTING_H
