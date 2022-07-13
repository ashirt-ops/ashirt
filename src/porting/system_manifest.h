#pragma once

#include <QObject>
#include <QJsonObject>

#include "helpers/file_helpers.h"
#include "helpers/jsonhelpers.h"
#include "helpers/screenshot.h"
#include "appconfig.h"
#include "db/databaseconnection.h"
#include "evidence_manifest.h"
#include "models/codeblock.h"
#include "system_porting_options.h"

namespace porting {
  class SystemManifest : public QObject {
    Q_OBJECT

  public:
    SystemManifest() {}
    ~SystemManifest(){}

    /**
    * @brief readManifest parses the the system.json (as provided by the caller) into a complete SystemManifest
    * @param pathToExportFile the location of the system.json file
    * @return the completed SystemManifest
    */
    static SystemManifest* readManifest(const QString& pathToExportFile);

    /// serialize converts the internal state of the system manifest into a json object
    static QJsonObject serialize(const SystemManifest& src);
    /// deserialize converts a json object into a system manifest instance
    static SystemManifest* deserialize(const QJsonObject& o);

    /**
    * @brief applyManifest takes the given manifest object (and options), and begins merging that data with the running system
    * @param options switches to control what gets imported
    * @param systemDb The currently running/system database
    */
    void applyManifest(SystemManifestImportOptions options, DatabaseConnection* systemDb);

    /**
    * @brief exportManifest starts the long process of copying config and evidence into the specified directory.
    * @param db the connection to the primary database
    * @param outputDirPath the path to the expected export directory. Files will be placed under this directory
    * (not wrapped in another directory)
    * @param options exporting options (e.g. do you want to copy both evidence *and* config
    */
    void exportManifest(DatabaseConnection* db, const QString& outputDirPath,
                          const SystemManifestExportOptions& options);

    /// os is the operating system associated with the originating export
    QString os;
    /// dbPath is the (relative) path to the database file from the originating export
    QString dbPath;
    /// configPath is the (relative) path to the config file from the originating export
    QString configPath;
    /// serversPath is currently unused
    QString serversPath;
    /// evidenceManifestPath is the (relative) path to the evidence manifest file from the originating export
    QString evidenceManifestPath;

  signals:
    /// onReady fires when the breadth of the import/export is known to let the caller know that real work is starting
    void onReady(quint64 numFilesToProcess);
    /// onFileProcessed fires when an evidence file is copied during import or export
    void onFileProcessed(quint64 runningCount);
    /// onComplete fires when the entire import/export is finished
    void onComplete();
    /// onComplete fires when the export has an error
    void onExportError(QString errorString);
    /// onCopyFileError fires when a file cannot be copied during import or export
    void onCopyFileError(QString srcPath, QString dstPath, const QString& errStr);
    /// onStatusUpdate fires when the system moves between import/export phases
    void onStatusUpdate(QString text);

  private:
    /// contentSensitiveFilename returns a (random) filename for the given content type. This, in
    /// turn, relies on the underlying type to provide a sensible value. If no match is found, then
    /// "ashirt_unknown_type_XXXXXX.bin" (X's will be replaced with random characters) is returned
    /// instead.
    /// @see FileHelpers::randomFilename
    static QString contentSensitiveFilename(const QString& contentType);

    /// contentSensitiveExtension returns a file extension for the given content type. This, in turn,
    /// relies on the underlying type to provide a sensible value. If no match is found, then ".bin"
    /// is returned instead
    static QString contentSensitiveExtension(const QString& contentType);

    /**
    * @brief migrateConfig imports the config file associated with the started import
    */
    void migrateConfig();

    /**
    * @brief migrateDb imports all of the database and evidence files associated with the started import
    * emits onStatusUpdate signal for periodic progress updates
    * emits onCopyFileError signal if there is an issue copying evidence files
    * emits onFileProcessed for each file processed
    * @param systemDb a pointer to the "standard" system database/running database
    * @throws QSqlError
    */
    void migrateDb(DatabaseConnection* systemDb);

    /// pathToFile is a small helper method to combine the absolute path to the manifest with the relative
    /// path to the given filename. The result is an absolute path to the given file
    QString pathToFile(const QString& filename);

    /**
    * @brief copyEvidence will iteratively copy all evidence files provided to the indicated path. Files are renamed
    * to avoid any name collisions. Files are namespaced into givenPath/evidence
    * This emits a onCopyFileError signal if there is an issue copying files
    * This emits a onFileProcessed signal when the attempted copy completes (so you may get an error _and_ processed signal on the same file. Error will be first)
    * @param baseExportPath The path to the desired export directory
    * @param allEvidence a vector of evidence _data_ to export (files will be found and read from within this function)
    * @return an EvidenceManifest listing all of the files copied, and their new names.
    */
    porting::EvidenceManifest copyEvidence(const QString& baseExportPath,
                                             QList<model::Evidence> allEvidence);

    /// pathToManifest is the (absolute) path to the system manifest file from the originating export
    QString m_pathToManifest;
    inline static const QString m_fileTemplate = QStringLiteral("%1/%2");
  };
}
