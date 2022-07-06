#pragma once

#include <QJsonArray>
#include <QJsonObject>

#include "helpers/jsonhelpers.h"
#include "helpers/file_helpers.h"

namespace porting {

/**
 * @brief The EvidenceItem class is a simple object that records information about an exported
 * evidence item. It also knows how to encode and decode itself for exporting/importing purposes
 */
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
    o.insert(QStringLiteral("evidenceID"), item.evidenceID);
    o.insert(QStringLiteral("path"), item.exportPath);
    return o;
  }
  static EvidenceItem deserialize(QJsonObject o) {
    return EvidenceItem(o.value(QStringLiteral("evidenceID")).toInt(), o.value(QStringLiteral("path")).toString());
  }

 public:
  qint64 evidenceID = 0;
  QString exportPath;
};

/**
 * @brief The EvidenceManifest class is simply a wrapper around a collection of EvidenceItems. It
 * also provides a mechanism to encode and decode itself for exporting/importing purposes
 */
class EvidenceManifest {
 public:
  static QJsonArray serialize(EvidenceManifest manifest) {
    QJsonArray a;
    for (const EvidenceItem& evi : manifest.entries) {
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
  QList<EvidenceItem> entries;
};

}
