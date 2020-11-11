#include "system_manifest.h"

using namespace sync;

void SystemManifest::applyManifest(SystemManifestImportOptions options, DatabaseConnection* systemDb) {
  bool shouldMigrateConfig = options.importConfig && !configPath.isEmpty();
  bool shouldMigrateDb = options.importDb == SystemManifestImportOptions::Merge && !dbPath.isEmpty();

  if (shouldMigrateConfig) {
    migrateConfig();
  }

  if (shouldMigrateDb) {
    migrateDb(systemDb);
  }
}

void SystemManifest::migrateConfig() {
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

void SystemManifest::migrateDb(DatabaseConnection* systemDb) {
  auto evidenceManifest = EvidenceManifest::deserialize(pathToFile(evidenceManifestPath));
  DatabaseConnection::withConnection(
      pathToFile(dbPath), "importDb", [this, evidenceManifest, systemDb](DatabaseConnection importDb) {
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

QString SystemManifest::pathToFile(QString filename) {
  return pathToManifest + "/" + filename;
}

QString SystemManifest::contentSensitiveExtension(QString contentType) {
  if (contentType == Codeblock::contentType()) {
    return Codeblock::extension();
  }
  else if(contentType == Screenshot::contentType()) {
    return Screenshot::extension();
  }
  else {
    return ".bin";
  }
}

QString SystemManifest::contentSensitiveFilename(QString contentType) {
  if (contentType == Codeblock::contentType()) {
    return Codeblock::mkName();
  }
  else if(contentType == Screenshot::contentType()) {
    return Screenshot::mkName();
  }
  else {
    return FileHelpers::randomFilename("ashirt_unknown_type_XXXXXX.bin");
  }
}

SystemManifest SystemManifest::readManifest(QString pathToExportFile) {
  auto content = FileHelpers::readFile(pathToExportFile);
  SystemManifest manifest = parseJSONItem<SystemManifest>(content, &SystemManifest::deserialize);
  manifest.pathToManifest = FileHelpers::getDirname(pathToExportFile);

  return manifest;
}

void SystemManifest::exportManifest(DatabaseConnection* db, QString outputDirPath, const SystemManifestExportOptions& options) {
  if (!options.includesAnything()) {
    return;
  }

  bool success = FileHelpers::mkdirs(outputDirPath);
  if (!success) {
    return;
  }

  os = QSysInfo::kernelType(); // may need to check possible answers, or maybe just compare to new system value?

  QString basePath = QDir(outputDirPath).path();

  if (options.exportConfig) {
    configPath = "config.json";
    AppConfig::getInstance().writeConfig(basePath + "/" + configPath);
  }

  if (options.exportDb) {
    dbPath = "db.sqlite";
    evidenceManifestPath = "evidence.json";

    auto allEvidence = DatabaseConnection::createEvidenceExportView(basePath + "/" + dbPath, EvidenceFilters(), db);
    sync::EvidenceManifest evidenceManifest = copyEvidence(basePath, allEvidence);

    // write evidence manifest
    FileHelpers::writeFile(basePath + "/" + evidenceManifestPath,
                           QJsonDocument(EvidenceManifest::serialize(evidenceManifest)).toJson());
  }

  QString exportPath = basePath + "/system.json";
  this->pathToManifest = exportPath;
  FileHelpers::writeFile(exportPath, QJsonDocument(serialize(*this)).toJson());
}

sync::EvidenceManifest SystemManifest::copyEvidence(QString baseExportPath, std::vector<model::Evidence> allEvidence) {
  QString relativeEvidenceDir = "evidence";
  FileHelpers::mkdirs(baseExportPath + "/" + relativeEvidenceDir);

  sync::EvidenceManifest evidenceManifest;
  for (auto evi : allEvidence) {
    QString randPart = "??????????";
    QString filenameTemplate = QString("ashirt_evidence_%1.%2").arg(randPart).arg(contentSensitiveExtension(evi.contentType));
    QString newName = FileHelpers::randomFilename(filenameTemplate, randPart);
    auto item = sync::EvidenceItem(evi.id, relativeEvidenceDir + "/" + newName);
    FileHelpers::copyFile(evi.path, baseExportPath + "/" + item.exportPath);
    evidenceManifest.entries.push_back(item);
  }
  return evidenceManifest;
}

QJsonObject SystemManifest::serialize(const SystemManifest& src) {
  QJsonObject o;
  o.insert("operatingSystem", src.os);
  o.insert("databasePath", src.dbPath);
  o.insert("configPath", src.configPath);
  o.insert("serversPath", src.serversPath);
  o.insert("evidenceManifestPath", src.evidenceManifestPath);
  return o;
}

SystemManifest SystemManifest::deserialize(QJsonObject o) {
  SystemManifest manifest;
  manifest.os = o.value("operatingSystem").toString();
  manifest.dbPath = o.value("databasePath").toString();
  manifest.configPath = o.value("configPath").toString();
  manifest.serversPath = o.value("serversPath").toString();
  manifest.evidenceManifestPath = o.value("evidenceManifestPath").toString();
  return manifest;
}
