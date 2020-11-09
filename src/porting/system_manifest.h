#ifndef SYNC_SYSTEM_MANIFEST_H
#define SYNC_SYSTEM_MANIFEST_H

#include <QString>
#include <QFile>
#include <QJsonObject>

#include "helpers/file_helpers.h"
#include "helpers/jsonhelpers.h"
#include "helpers/screenshot.h"
#include "appconfig.h"
#include "appsettings.h"
#include "db/databaseconnection.h"
#include "evidence_manifest.h"
#include "models/codeblock.h"

#include "system_porting_options.h"

namespace sync {

class SystemManifest {

 public:
  static SystemManifest buildManifest(DatabaseConnection* db, QString outputDirPath, const SystemManifestExportOptions& options) {
    auto rtn = SystemManifest();
    if (!options.includesAnything()) {
      return rtn;
    }

    bool success = FileHelpers::mkdirs(outputDirPath);
    if (!success) {
      return rtn;
    }

    rtn.os = QSysInfo::kernelType(); // may need to check possible answers, or maybe just compare to new system value?

    QString basePath = QDir(outputDirPath).path();
    try {
      // copy config
      if (options.exportConfig) {
        rtn.configPath = "config.json";
        QString cfgPath = basePath + "/" + rtn.configPath;
        AppConfig::getInstance().writeConfig(cfgPath);
      }

      // copy database
      if (options.exportDb) {
        rtn.dbPath = "db.sqlite";
        QString dbPath = basePath + "/" + rtn.dbPath;

        // FileHelpers::copyFile(Constants::dbLocation(), dbPath);  // faster copy, no fine tuning

        // slower copy, abilty to select specific details
        auto allEvidence = DatabaseConnection::createEvidenceExportView(dbPath, EvidenceFilters(), db);

        // Copy evidence content
        QString evidenceBase = "evidence";
        FileHelpers::mkdirs(basePath + "/" + evidenceBase);

        sync::EvidenceManifest evidenceManifest;
        for (auto evi : allEvidence) {
          QString newName = FileHelpers::randomFilename("ashirt_evidence_??????????." + contentSensitiveExtension(evi.contentType), "??????????");
          auto item = sync::EvidenceItem(evi.id, evidenceBase + "/" + newName);
          FileHelpers::copyFile(evi.path, basePath + "/" + item.exportPath);
          evidenceManifest.entries.push_back(item);
        }

        // write evidence manifest
        rtn.evidenceManifestPath = "evidence.json";
        QString evidenceManifestPath = basePath + "/" + rtn.evidenceManifestPath;
        FileHelpers::writeFile(evidenceManifestPath, QJsonDocument(EvidenceManifest::serialize(evidenceManifest)).toJson());
      }

      QString exportPath = basePath + "/system.json";
      rtn.pathToManifest = exportPath;
      FileHelpers::writeFile( exportPath, QJsonDocument(serialize(rtn)).toJson() );
    }
    catch (const FileError &e) {
      std::cerr << "File error doing buildManifest: " << e.what() << std::endl;
    }
    catch (const QSqlError &e) {
      std::cerr << "SQL error doing buildManifest: " << e.text().toStdString() << std::endl;
    }


    return rtn;
  }

  static SystemManifest readManifest(QString pathToExportFile) {
    SystemManifest manifest;

    try {
      auto content = FileHelpers::readFile(pathToExportFile);
      manifest = parseJSONItem<SystemManifest>(content, &SystemManifest::deserialize);
      manifest.pathToManifest = FileHelpers::getDirname(pathToExportFile);
    }
    catch (const FileError& e) {
      std::cout << "Got an exception: " << e.what() << std::endl;
    }

    return manifest;
  }

  static QString contentSensitiveFilename(QString contentType) {
    if (contentType == Codeblock::contentType()) {
      return Codeblock::mkName();
    }
    else if(contentType == Screenshot::contentType()) {
      return Screenshot::mkName();
    }
    else {
      return FileHelpers::randomFilename("ashirt_unknown_type.bin");
    }
  }

  static QString contentSensitiveExtension(QString contentType) {
    if (contentType == Codeblock::contentType()) {
      return Codeblock::extension();
    }
    else if(contentType == Screenshot::contentType()) {
      return Screenshot::extension();
    }
    else {
      return FileHelpers::randomFilename("bin");
    }
  }

  static QJsonObject serialize(SystemManifest src) {
    QJsonObject o;
    o.insert("operatingSystem", src.os);
    o.insert("databasePath", src.dbPath);
    o.insert("configPath", src.configPath);
    o.insert("serversPath", src.serversPath);
    o.insert("evidenceManifestPath", src.evidenceManifestPath);
    return o;
  }

  static SystemManifest deserialize(QJsonObject o) {
    SystemManifest manifest;
    manifest.os = o.value("operatingSystem").toString();
    manifest.dbPath = o.value("databasePath").toString();
    manifest.configPath = o.value("configPath").toString();
    manifest.serversPath = o.value("serversPath").toString();
    manifest.evidenceManifestPath = o.value("evidenceManifestPath").toString();
    return manifest;
  }

 public:
  void applyManifest(SystemManifestImportOptions options, DatabaseConnection* systemDb) {
    bool shouldMigrateConfig = options.importConfig && !configPath.isEmpty();
    bool shouldMigrateDb = options.importDb == SystemManifestImportOptions::Merge && !dbPath.isEmpty();

    try {
      if (shouldMigrateConfig) {
        migrateConfig();
      }

      if (shouldMigrateDb) {
        migrateDb(systemDb);
      }
    }
    catch (const FileError& e) {
      std::cout << "Got an exception: " << e.what() << std::endl;
    }
  }


 private:
  void migrateConfig() {
    auto data = FileHelpers::readFile(pathToFile(configPath));
    parseJSONItem<QString>(data, [](QJsonObject src) {
      for(QString key : src.keys()) {
        src.remove("evidenceRepo"); // removing evidenceRepo, as we never want to replace what the user has set there.

        // only opting to migrate connection settings, given that translating other options may
        // cause problems (especially if migrating between oses)
        if (key != "accessKey" && key != "secretKey" && key != "apiURL") {
          src.remove(key);
        }
      }
      AppConfig::getInstance().applyConfig(src);
      return "";
    });
    AppConfig::getInstance().writeConfig(); // save updated config
  }

  void migrateDb(DatabaseConnection* systemDb) {
    auto evidenceManifest = EvidenceManifest::deserialize(pathToFile(evidenceManifestPath));
    DatabaseConnection::withConnection(pathToFile(dbPath), "importDb",
                                       [this, evidenceManifest, systemDb](DatabaseConnection importDb) {
      // migrate evidence (make new file, copy content, create db entry)
      for (auto item : evidenceManifest.entries) {
        auto importRecord = importDb.getEvidenceDetails(item.evidenceID);
        if (importRecord.id == 0) {
          continue; // in the odd situation that evidence doesn't match up, just skip it
        }
        QString newEvidencePath = AppConfig::getInstance().evidenceRepo + "/" +
                                  importRecord.operationSlug + "/" +
                                  contentSensitiveFilename(importRecord.contentType);
        QString newEviPathLastFour = newEvidencePath.right(4);

        auto fullFileExportPath = pathToManifest + "/" + item.exportPath;
        auto success = FileHelpers::copyFile(fullFileExportPath, newEvidencePath, true);
        importRecord.path = newEvidencePath;
        qint64 evidenceID = systemDb->createFullEvidence(importRecord);
        systemDb->setEvidenceTags(importRecord.tags, evidenceID);
      }
    });
  }

  QString pathToFile(QString filename) { return pathToManifest + "/" + filename; }


 public:
  QString os = "";
  QString dbPath = "";
  QString configPath = "";
  QString serversPath = "";
  QString evidenceManifestPath = "";

 private:
  QString pathToManifest = "";
};
}

#endif // SYNC_SYSTEM_MANIFEST_H

