#ifndef CONFIG_SERVER_H
#define CONFIG_SERVER_H

#include "appsettings.h"
#include "helpers/file_helpers.h"
#include "server_item.h"

/**
 * @brief The ServerSet class represents a standard set of functionality for any given server
 * configuration "file". This comes on the heel of ConfigV2, and as such, ServersV1 is reserved,
 * and numbering of servers implementing ServerSet begins at 2.
 */
class ServerSet {
 public:
  virtual ~ServerSet() {};

 public:
  virtual void writeToFile(QString filepath) {
    QByteArray data = toFileEncoding();
    FileHelpers::writeFile(filepath, data);
  }

  virtual void readFromServers(QString filepath) {
    readResult = FileHelpers::readFileNoError(filepath);
    if(readResult.success) {
      parseConfigData(readResult.data);
      setLoadedPath(filepath);
      return;
    }
    else if(!readResult.fileExists){
      makeDefaultServerset();
      setLoadedPath(filepath);
      writeToFile(filepath);
    }
    else {
      valid = false;
    }
  }

  bool isValid() {
    return valid;
  }
  FileReadResult getReadResult() {
    return readResult;
  }

 public:
  virtual ServerItem getServerByUuid(const QString& uuid) = 0;
  virtual std::vector<ServerItem> getServers(bool includeDeleted) = 0;
  virtual ServerItem deleteServer(const QString& uuid, bool undelete=false) = 0;
  virtual void addServer(ServerItem item) = 0;
  virtual void updateServer(ServerItem item) = 0;


 protected:
  virtual void parseConfigData(const QByteArray& data) = 0;
  virtual QByteArray toFileEncoding() = 0;
  virtual QString getLoadedPath() = 0;
  virtual void setLoadedPath(QString path) = 0;
  virtual void makeDefaultServerset() {}

 private:
  bool valid = true;
  FileReadResult readResult;
};

#endif // CONFIG_SERVER_H
