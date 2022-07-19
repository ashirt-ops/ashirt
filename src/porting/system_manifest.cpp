#include "system_manifest.h"

#include "helpers/string_helpers.h"

using namespace porting;

void SystemManifest::applyManifest(SystemManifestImportOptions options, DatabaseConnection* systemDb)
{
    bool shouldMigrateConfig = options.importConfig && !configPath.isEmpty();
    bool shouldMigrateDb = options.importDb == SystemManifestImportOptions::Merge && !dbPath.isEmpty();

    if (shouldMigrateConfig) {
        Q_EMIT onStatusUpdate(tr("Importing Settings"));
        AppConfig::importConfig(configPath);
    }

    if (shouldMigrateDb) {
        migrateDb(systemDb);
    }
    Q_EMIT onComplete();
}

void SystemManifest::migrateDb(DatabaseConnection* systemDb)
{
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
            if (importRecord.id == 0)
                continue; // in the odd situation that evidence doesn't match up, just skip it
            QString newEvidencePath = QStringLiteral("%1/%2/%3")
                    .arg(AppConfig::value(CONFIG::EVIDENCEREPO)
                         , importRecord.operationSlug
                         , contentSensitiveFilename(importRecord.contentType));

            auto fullFileExportPath = m_fileTemplate.arg(m_pathToManifest, item.exportPath);
            auto parentDir = FileHelpers::getDirname(newEvidencePath);
            if (!QDir().exists(parentDir))
                QDir().mkpath(parentDir);
            QFile srcFile(fullFileExportPath);
            srcFile.copy(newEvidencePath);
            if (srcFile.error() != QFileDevice::NoError) {
                Q_EMIT onCopyFileError(
                            fullFileExportPath, newEvidencePath,
                            QStringLiteral("Unable to write to file: %1\n%2").arg(newEvidencePath, srcFile.error()));
                return;
            }

            importRecord.path = newEvidencePath;
            qint64 evidenceID = systemDb->createFullEvidence(importRecord);
            systemDb->setEvidenceTags(importRecord.tags, evidenceID);
        }
        Q_EMIT onFileProcessed(evidenceManifest.entries.size()); // update the full set now that this is complete
    });
}

QString SystemManifest::pathToFile(const QString& filename)
{
    return m_fileTemplate.arg(m_pathToManifest, filename);
}

QString SystemManifest::contentSensitiveExtension(const QString& contentType)
{
    if (contentType == Codeblock::contentType())
        return Codeblock::extension();
    if(contentType == Screenshot::contentType())
        return Screenshot::extension();
    return QStringLiteral(".bin");
}

QString SystemManifest::contentSensitiveFilename(const QString& contentType)
{
    if (contentType == Codeblock::contentType())
        return Codeblock::mkName();
    if(contentType == Screenshot::contentType())
        return Screenshot::mkName();
    return QStringLiteral("ashirt_unknown_type_%1.bin").arg(StringHelpers::randomString());
}

SystemManifest* SystemManifest::readManifest(const QString& pathToExportFile)
{
    auto content = FileHelpers::readFile(pathToExportFile);
    auto manifest = parseJSONItem<SystemManifest*>(content, &SystemManifest::deserialize);
    manifest->m_pathToManifest = FileHelpers::getDirname(pathToExportFile);
    return manifest;
}

void SystemManifest::exportManifest(DatabaseConnection* db, const QString& outputDirPath,
                                    const SystemManifestExportOptions& options)
{
    if (!options.includesAnything() || outputDirPath.isEmpty())
        return;

    if (!QDir().mkpath(outputDirPath))
        return;

    os = QSysInfo::kernelType(); // may need to check possible answers, or maybe just compare to new system value?
    QString basePath = QDir(outputDirPath).path();
    if (options.exportConfig) {
        Q_EMIT onStatusUpdate(tr("Exporting settings"));
        configPath = QStringLiteral("config.json");
        AppConfig::exportConfig(m_fileTemplate.arg(basePath, configPath));
    }

    if (options.exportDb) {
        Q_EMIT onStatusUpdate(tr("Exporting Evidence"));
        dbPath = QStringLiteral("db.sqlite");
        evidenceManifestPath = QStringLiteral("evidence.json");
        auto allEvidence = DatabaseConnection::createEvidenceExportView(m_fileTemplate.arg(basePath, dbPath), EvidenceFilters(), db);
        Q_EMIT onReady(allEvidence.size());
        porting::EvidenceManifest evidenceManifest = copyEvidence(basePath, allEvidence);
        // write evidence manifest
        FileHelpers::writeFile(m_fileTemplate.arg(basePath, evidenceManifestPath),
                               QJsonDocument(EvidenceManifest::serialize(evidenceManifest)).toJson());
    }

    m_pathToManifest = QStringLiteral("%1/system.json").arg(basePath);
    if(FileHelpers::writeFile(m_pathToManifest, QJsonDocument(serialize(*this)).toJson()))
        Q_EMIT onComplete();
    else
        Q_EMIT onExportError(QStringLiteral("Error On Exporting manifest"));
}

porting::EvidenceManifest SystemManifest::copyEvidence(const QString& baseExportPath,
                                                       QList<model::Evidence> allEvidence)
{
    QString relativeEvidenceDir = QStringLiteral("evidence");
    QDir().mkpath(m_fileTemplate.arg(baseExportPath, relativeEvidenceDir));

    porting::EvidenceManifest evidenceManifest;
    for (size_t evidenceIndex = 0; evidenceIndex < allEvidence.size(); evidenceIndex++) {
        auto evi = allEvidence.at(evidenceIndex);
        auto newName = QStringLiteral("ashirt_evidence_%1.%2")
                .arg(StringHelpers::randomString(10), contentSensitiveExtension(evi.contentType));
        auto item = porting::EvidenceItem(evi.id, m_fileTemplate.arg(relativeEvidenceDir, newName));
        auto dstPath = m_fileTemplate.arg(baseExportPath, item.exportPath);
        QFile srcFile(evi.path);
        if(!srcFile.copy(dstPath))
            Q_EMIT onCopyFileError(evi.path, dstPath, srcFile.errorString());
        else
            evidenceManifest.entries.append(item);
        Q_EMIT onFileProcessed(evidenceIndex + 1);
    }
    return evidenceManifest;
}

QJsonObject SystemManifest::serialize(const SystemManifest& src)
{
    QJsonObject o;
    o.insert(QStringLiteral("operatingSystem"), src.os);
    o.insert(QStringLiteral("databasePath"), src.dbPath);
    o.insert(QStringLiteral("configPath"), src.configPath);
    o.insert(QStringLiteral("serversPath"), src.serversPath);
    o.insert(QStringLiteral("evidenceManifestPath"), src.evidenceManifestPath);
    return o;
}

SystemManifest* SystemManifest::deserialize(const QJsonObject& o)
{
    auto manifest = new SystemManifest;
    manifest->os = o.value(QStringLiteral("operatingSystem")).toString();
    manifest->dbPath = o.value(QStringLiteral("databasePath")).toString();
    manifest->configPath = o.value(QStringLiteral("configPath")).toString();
    manifest->serversPath = o.value(QStringLiteral("serversPath")).toString();
    manifest->evidenceManifestPath = o.value(QStringLiteral("evidenceManifestPath")).toString();
    return manifest;
}
