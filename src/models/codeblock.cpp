#include "codeblock.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QVariant>
#include <utility> 
#include "helpers/file_helpers.h"
#include "helpers/jsonhelpers.h"

static Codeblock fromJson(QJsonObject obj) {
  Codeblock rtn;
  rtn.content = obj["content"].toVariant().toString();
  rtn.subtype = obj["contentSubtype"].toString();
  QJsonObject meta = obj["metadata"].toObject();
  if (!meta.empty()) {
    rtn.source = meta["source"].toString();
  }

  return rtn;
}

Codeblock::Codeblock() = default;

Codeblock::Codeblock(QString content) {
  this->filename =
      FileHelpers::randomFilename(FileHelpers::pathToEvidence() + "ashirt_codeblock_XXXXXX.json");
  this->content = std::move(content);
  this->subtype = "";
  this->source = "";
}

void Codeblock::saveCodeblock(Codeblock codeblock) {
  FileHelpers::writeFile(codeblock.filename, codeblock.encode());
}

Codeblock Codeblock::readCodeblock(const QString& filepath) {
  QByteArray data = FileHelpers::readFile(filepath);

  Codeblock rtn = parseJSONItem<Codeblock>(data, fromJson);
  rtn.filename = filepath;
  return rtn;
}

QByteArray Codeblock::encode() {
  QJsonObject root;

  root.insert("contentSubtype", subtype);
  root.insert("content", content);

  QJsonObject metadata;

  if (source != "") {
    metadata.insert("source", source);
  }

  if (!metadata.empty()) {
    root.insert("metadata", metadata);
  }
  return QJsonDocument(root).toJson();
}
