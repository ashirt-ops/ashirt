#ifndef SYSTEM_OPTIONS_H
#define SYSTEM_OPTIONS_H

namespace porting {

class SystemManifestExportOptions {

 public:
  bool includesAnything() const {
    return exportConfig || exportDb;
  }

 public:
  bool exportConfig = true;
  bool exportDb = true;
};

class SystemManifestImportOptions {
 public:
  enum ImportAction {
    None = 0,
    Merge = 1,
    };

  bool includesAnything() const {
    return importConfig || (importDb != None);
  }

 public:
  bool importConfig = true;
  ImportAction importDb = Merge;
};

}

#endif // SYSTEM_OPTIONS_H
