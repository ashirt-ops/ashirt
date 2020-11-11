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
  SystemManifest() {}

  /**
   * @brief readManifest parses the the system.json (as provided by the caller) into a complete SystemManifest
   * @param pathToExportFile the location of the system.json file
   * @return the completed SystemManifest
   */
  static SystemManifest readManifest(QString pathToExportFile);

 public:
  static QJsonObject serialize(const SystemManifest& src);

  static SystemManifest deserialize(QJsonObject o);

 private:
  /// contentSensitiveFilename returns a (random) filename for the given content type. This, in turn,
  /// relies on the underlying type to provide a sensible value. If no match is found, then
  /// "ashirt_unknown_type_XXXXXX.bin" (X's will be replaced with random characters) is returned instead.
  /// @see FileHelpers::randomFilename
  static QString contentSensitiveFilename(QString contentType);

  /// contentSensitiveExtension returns a file extension for the given content type. This, in turn,
  /// relies on the underlying type to provide a sensible value. If no match is found, then ".bin"
  /// is returned instead
  static QString contentSensitiveExtension(QString contentType);



 public:
  /**
   * @brief applyManifest
   * @param options
   * @param systemDb
   * @throws FileError
   * @throws QSqlError
   */
  void applyManifest(SystemManifestImportOptions options, DatabaseConnection* systemDb);

  /**
   * @brief exportManifest starts the long process of copying config and evidence into the specified directory.
   * @param db the connection to the primary database
   * @param outputDirPath the path to the expected export directory. Files will be placed under this directory
   * (not wrapped in another directory)
   * @param options exporting options (e.g. do you want to copy both evidence *and* config
   * @throws FileError if there is a problem with the given directory, or with a generated filename
   * @throws QSqlError if there is an issue accessing the system database, or the copied database
   */
  void exportManifest(DatabaseConnection* db, QString outputDirPath, const SystemManifestExportOptions& options);


 private:
  /**
   * @brief migrateConfig
   * @throws FileError
   */
  void migrateConfig();
  /**
   * @brief migrateDb
   * @param systemDb
   * @throws QSqlError
   */
  void migrateDb(DatabaseConnection* systemDb);
  QString pathToFile(QString filename);

  /**
   * @brief copyEvidence
   * @param baseExportPath
   * @param allEvidence
   * @throws QSqlError
   * @return
   */
  sync::EvidenceManifest copyEvidence(QString baseExportPath, std::vector<model::Evidence> allEvidence);


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

