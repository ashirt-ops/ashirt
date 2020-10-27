#ifndef MULTI_SERVER_MIGRATION_H
#define MULTI_SERVER_MIGRATION_H

#include "migrations/migration.h"
#include "appconfig.h"
#include "helpers/constants.h"

class MultiServerMigration : public Migration {

 public:

  QString migrationName() override {
    return "multi-server";
  }

 protected:
  bool doMigration(DatabaseConnection* conn) override {
    ConfigV1* config = AppConfig::getInstance().asConfigV1();
    if (config == nullptr) {
      return false;
    }
    conn->updateFullServerDetails(Constants::defaultServerName(),
                                  config->accessKey(),
                                  config->secretKey(),
                                  config->apiURL(),
                                  Constants::legacyServerUuid());
    return true;
  }
};

#endif // MULTI_SERVER_MIGRATION_H
