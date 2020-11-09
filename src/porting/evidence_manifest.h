#ifndef SYNC_EVIDENCE_MANIFEST_H
#define SYNC_EVIDENCE_MANIFEST_H

#include <QString>
#include <QJsonArray>
#include <QJsonObject>

#include <vector>

#include "helpers/jsonhelpers.h"
#include "helpers/file_helpers.h"

namespace sync {

class EvidenceItem {
 public:
  EvidenceItem(){}
  EvidenceItem(qint64 id, QString path) {
    this->evidenceID = id;
    this->exportPath = path;
  }

 public:
  static QJsonObject serialize(EvidenceItem item) {
    QJsonObject o;
    o.insert("evidenceID", item.evidenceID);
    o.insert("path", item.exportPath);
    return o;
  }
  static EvidenceItem deserialize(QJsonObject o) {
    return EvidenceItem(o.value("evidenceID").toInt(), o.value("path").toString());
  }

 public:
  qint64 evidenceID = 0;
  QString exportPath = "";
};

class EvidenceManifest {
 public:
  static QJsonArray serialize(EvidenceManifest manifest) {
    QJsonArray a;
    for (EvidenceItem evi : manifest.entries) {
      a.append(EvidenceItem::serialize(evi));
    }
    return a;
  }

  static EvidenceManifest deserialize(QString filepath) {
    EvidenceManifest manifest;
    manifest.entries = parseJSONList<EvidenceItem>(
        FileHelpers::readFile(filepath), &EvidenceItem::deserialize);
    return manifest;
  }

 public:
  std::vector<EvidenceItem> entries;
};

}

#endif // SYNC_EVIDENCE_MANIFEST_H
