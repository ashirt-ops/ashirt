#include "codeblock.h"

#include <QJsonDocument>
#include <QJsonObject>

#include "helpers/file_helpers.h"
#include "helpers/string_helpers.h"
#include "helpers/system_helpers.h"
#include "helpers/jsonhelpers.h"

static Codeblock fromJson(QJsonObject obj)
{
    Codeblock rtn;
    rtn.content = obj.value(QStringLiteral("content")).toVariant().toString();
    rtn.subtype = obj.value(QStringLiteral("contentSubtype")).toString();
    QJsonObject meta = obj.value(QStringLiteral("metadata")).toObject();
    if (!meta.empty())
        rtn.source = obj.value(QStringLiteral("source")).toString();
    return rtn;
}

Codeblock::Codeblock(QString content)
    : filename(SystemHelpers::pathToEvidence() + Codeblock::mkName())
    , content(std::move(content))
{ }

QString Codeblock::mkName()
{
    return QStringLiteral("ashirt_codeblock_%1.%2").arg(StringHelpers::randomString(), extension());
}

QString Codeblock::extension()
{
    return QStringLiteral("json");
}

bool Codeblock::saveCodeblock(Codeblock codeblock)
{
    return FileHelpers::writeFile(codeblock.filename, codeblock.encode());
}

Codeblock Codeblock::readCodeblock(const QString& filepath)
{
    Codeblock rtn = parseJSONItem<Codeblock>(FileHelpers::readFile(filepath), fromJson);
    rtn.filename = filepath;
    return rtn;
}

QString Codeblock::contentType()
{
    return QStringLiteral("codeblock");
}

QByteArray Codeblock::encode()
{
    QJsonObject root;
    root.insert(QStringLiteral("contentSubtype"), subtype);
    root.insert(QStringLiteral("content"), content);

    QJsonObject metadata;
    if (!source.isEmpty())
        metadata.insert(QStringLiteral("source"), source);
    if (!metadata.empty())
        root.insert(QStringLiteral("metadata"), metadata);
    return QJsonDocument(root).toJson();
}
