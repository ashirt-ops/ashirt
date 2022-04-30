#include "system_manifest.h"

using namespace porting;

void SystemManifest::applyManifest(SystemManifestImportOptions options, DatabaseConnection* systemDb) {
  bool shouldMigrateConfig = options.importConfig && !configPath.isEmpty();
  bool shouldMigrateDb = options.importDb == SystemManifestImportOptions::Merge && !dbPath.isEmpty();

  if (shouldMigrateConfig) {
    Q_EMIT onStatusUpdate(tr("Importing Settings"));
    migrateConfig();
  }

  if (shouldMigrateDb) {
    migrateDb(systemDb);
  }
  Q_EMIT onComplete();
}

void SystemManifest::migrateConfig() {
  auto data = FileHelpers::readFile(pathToFile(configPath));
  parseJSONItem<QString>(data, [](QJsonObject src) {
    for(const QString& key : src.keys()) {
      src.remove(QStringLiteral("evidenceRepo")); // removing evidenceRepo, as we never want to replace what the user has set there.

      // only opting to migrate connection settings, given that translating other options may
      // cause problems (especially if migrating between oses)
      if (key != QStringLiteral("accessKey") && key != QStringLiteral("secretKey") && key != QStringLiteral("apiURL")) {
        src.remove(key);
      }
    }
    AppConfig::getInstance().applyConfig(src);
    return "";
  });
  AppConfig::getInstance().writeConfig(); // save updated config
}

void SystemManifest::migrateDb(DatabaseConnection* systemDb) {
  Q_EMIT onStatusUpdate(tr("Reading Exported Evidence"));
  auto evidenceManifest = EvidenceManifest::deserialize(pathToFile(evidenceManifestPath));
  Q_EMIT onReady(evidenceManifest.entries.size());
  DatabaseConnection::withConnection(
      pathToFile(dbPath), QStringLiteral("importDb"), [this, evidenceManifest, systemDb](DatabaseConnection importDb) {
        Q_EMIT onStatusUpdate(tr("Importing evidence"));
        for (size_t entryIndex = 0; entryIndex < evidenceManifest.entries.size(); entryIndex++) {
          Q_EMIT onFileProcessed(entryIndex); // this only makes sense on the 2nd+ iteration, but this works since indexes start at 0
          auto item = evidenceManifest.entries.at(entryIndex);
          auto importRecord = importDb.getEvidenceDetails(item.evidenceID);
          if (importRecord.id == 0) {
            continue; // in the odd situation that evidence doesn't match up, just skip it
          }
          QString newEvidencePath = QStringLiteral("%1/%2/%3")
                        .arg(AppConfig::getInstance().evidenceRepo
                         , importRecord.operationSlug
                         , contentSensitiveFilename(importRecord.contentType));

          auto fullFileExportPath = QStringLiteral("%1/%2").arg(pathToManifest, item.exportPath);
          auto copyResult = FileHelpers::copyFile(fullFileExportPath, newEvidencePath, true);

          if (!copyResult.success) {
            Q_EMIT onCopyFileError(fullFileExportPath, newEvidencePath,
                                 FileError::mkError(copyResult.file->errorString(), newEvidencePath, copyResult.file->error()));
          }

          importRecord.path = newEvidencePath;
          qint64 evidenceID = systemDb->createFullEvidence(importRecord);
          systemDb->setEvidenceTags(importRecord.tags, evidenceID);
       }
       Q_EMIT onFileProcessed(evidenceManifest.entries.size()); // update the full set now that this is complete
  });
}

QString SystemManifest::pathToFile(const QString& filename) {
  return QStringLiteral("%1/%2").arg(pathToManifest, filename);
}

QString SystemManifest::contentSensitiveExtension(const QString& contentType) {
  if (contentType == Codeblock::contentType()) {
    return Codeblock::extension();
  }
  else if(contentType == Screenshot::contentType()) {
    return Screenshot::extension();
  }
  return QStringLiteral(".bin");
}

QString SystemManifest::contentSensitiveFilename(const QString& contentType) {
  if (contentType == Codeblock::contentType()) {
    return Codeblock::mkName();
  }
  else if(contentType == Screenshot::contentType()) {
    return Screenshot::mkName();
  }
  return FileHelpers::randomFilename(QStringLiteral("ashirt_unknown_type_XXXXXX.bin"));
}

SystemManifest* SystemManifest::readManifest(const QString& pathToExportFile) {
  auto content = FileHelpers::readFile(pathToExportFile);
  auto manifest = parseJSONItem<SystemManifest*>(content, &SystemManifest::deserialize);
  manifest->pathToManifest = FileHelpers::getDirname(pathToExportFile);

  return manifest;
}

void SystemManifest::exportManifest(DatabaseConnection* db, const QString& outputDirPath,
                                    const SystemManifestExportOptions& options) {
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
    Q_EMIT onStatusUpdate(tr("Exporting settings"));
    configPath = QStringLiteral("config.json");
    AppConfig::getInstance().writeConfig(QStringLiteral("%1/%2").arg(basePath, configPath));
  }

  if (options.exportDb) {
    Q_EMIT onStatusUpdate(tr("Exporting Evidence"));
    dbPath = QStringLiteral("db.sqlite");
    evidenceManifestPath = QStringLiteral("evidence.json");

    auto allEvidence = DatabaseConnection::createEvidenceExportView(QStringLiteral("%1/%2").arg(basePath, dbPath), EvidenceFilters(), db);
    Q_EMIT onReady(allEvidence.size());
    porting::EvidenceManifest evidenceManifest = copyEvidence(basePath, allEvidence);

    // write evidence manifest
    FileHelpers::writeFile(QStringLiteral("%1/%2").arg(basePath, evidenceManifestPath),
                           QJsonDocument(EvidenceManifest::serialize(evidenceManifest)).toJson());
  }

  QString exportPath = QStringLiteral("%1/system.json").arg(basePath);
  this->pathToManifest = exportPath;
  FileHelpers::writeFile(exportPath, QJsonDocument(serialize(*this)).toJson());
  Q_EMIT onComplete();
}

porting::EvidenceManifest SystemManifest::copyEvidence(const QString& baseExportPath,
                                                       std::vector<model::Evidence> allEvidence) {
  QString relativeEvidenceDir = QStringLiteral("evidence");
  FileHelpers::mkdirs(QStringLiteral("%1/%2").arg(baseExportPath, relativeEvidenceDir));

  porting::EvidenceManifest evidenceManifest;
  for (size_t evidenceIndex = 0; evidenceIndex < allEvidence.size(); evidenceIndex++) {
    auto evi = allEvidence.at(evidenceIndex);
    QString randPart = QStringLiteral("??????????");
    auto filenameTemplate = QStringLiteral("ashirt_evidence_%1.%2")
                                .arg(randPart, contentSensitiveExtension(evi.contentType));
    QString newName = FileHelpers::randomFilename(filenameTemplate, randPart);
    auto item = porting::EvidenceItem(evi.id, relativeEvidenceDir + "/" + newName);
    auto dstPath = QStringLiteral("%1/%2").arg(baseExportPath, item.exportPath);
    auto copyResult = FileHelpers::copyFile(evi.path, dstPath);

    if (!copyResult.success) {
      Q_EMIT onCopyFileError(evi.path, dstPath,
                           FileError::mkError(copyResult.file->errorString(), dstPath, copyResult.file->error()));
    }
    else {
      evidenceManifest.entries.push_back(item);
    }
    Q_EMIT onFileProcessed(evidenceIndex + 1);
  }
  return evidenceManifest;
}

QJsonObject SystemManifest::serialize(const SystemManifest& src) {
  QJsonObject o;
  o.insert(QStringLiteral("operatingSystem"), src.os);
  o.insert(QStringLiteral("databasePath"), src.dbPath);
  o.insert(QStringLiteral("configPath"), src.configPath);
  o.insert(QStringLiteral("serversPath"), src.serversPath);
  o.insert(QStringLiteral("evidenceManifestPath"), src.evidenceManifestPath);
  return o;
}

SystemManifest* SystemManifest::deserialize(const QJsonObject& o) {
  auto manifest = new SystemManifest;
  manifest->os = o.value(QStringLiteral("operatingSystem")).toString();
  manifest->dbPath = o.value(QStringLiteral("databasePath")).toString();
  manifest->configPath = o.value(QStringLiteral("configPath")).toString();
  manifest->serversPath = o.value(QStringLiteral("serversPath")).toString();
  manifest->evidenceManifestPath = o.value(QStringLiteral("evidenceManifestPath")).toString();
  return manifest;
}
