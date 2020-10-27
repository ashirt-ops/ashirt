#include "migration.h"

#include <vector>

bool Migration::applyMigrations(DatabaseConnection* conn) {
  std::vector<Migration*> toApply = {
  };

  for (Migration* task : toApply) {
    if(!conn->hasAppliedSystemMigration(task->migrationName())) {
      std::cout << "Applying system migration: " << task->migrationName().toStdString() << std::endl;
      bool success = task->doMigration(conn);
      if(success) {
        conn->applySystemMigration(task->migrationName());
      }
      else {
        return false;
      }
    }
  }
  std::cout << "All system migrations applied" << std::endl;

  for (auto completed : toApply) {
    delete completed;
  }

  return true;
}
