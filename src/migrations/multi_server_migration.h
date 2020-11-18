#ifndef MULTI_SERVER_MIGRATION_H
#define MULTI_SERVER_MIGRATION_H

#include "migrations/migration.h"
#include "appconfig.h"
#include "helpers/constants.h"
#include "appservers.h"
#include "config/server_item.h"

class MultiServerMigration : public Migration {

 public:

  QString migrationName() override {
    return "multi-server";
  }

 protected:
  bool doMigration(DatabaseConnection* conn) override {
    Q_UNUSED(conn);
    ConfigV1* config = AppConfig::getInstance().asConfigV1();
    if (config == nullptr) {
      return false;
    }
    ServerItem item(Constants::legacyServerUuid(), Constants::defaultServerName(), config->accessKey(),
                    config->secretKey(), config->apiURL());

    AppServers::getInstance().addServer(item);

    return true;
  }
};

#endif // MULTI_SERVER_MIGRATION_H
