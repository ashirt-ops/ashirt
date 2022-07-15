// New AppConfig 2022, Chris Rizzitello
// Licensed under the terms of MIT. See LICENSE file in project root for terms.

#include <appconfig.h>

#include <QIODevice>
#include <QJsonParseError>
#include <QJsonObject>

AppConfig::AppConfig(QObject *parent)
    : QObject(parent)
    , appConfig(new QSettings(_configFile, AppConfig::JSON, this))
    , appSettings(new QSettings(this))
{
    validateConfig();
}

void AppConfig::validateConfig()
{
    //Remove Any invalid Keys and set or fix any keys needed.
    for (const auto &key : appConfig->allKeys()) {
        if(!_appConfigValidKeys.contains(key))
            appConfig->remove(key);
    }
    if (appConfig->value(CONFIG::EVIDENCEREPO).isNull())
        appConfig->setValue(CONFIG::EVIDENCEREPO, QStringLiteral("%1/ashirt/evidence").arg(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation)));
#ifdef Q_OS_MACOS
    if (appConfig->value(CONFIG::COMMAND_SCREENSHOT).isNull())
        appConfig->setValue(CONFIG::COMMAND_SCREENSHOT, QStringLiteral("screencapture -s %file"));
    if (appConfig->value(CONFIG::COMMAND_CAPTUREWINDOW).isNull())
        appConfig->setValue(CONFIG::COMMAND_CAPTUREWINDOW, QStringLiteral("screencapture -w %file"));
#endif
}

QString AppConfig::value(const QString &key)
{
    return get()->appConfig->value(key, QString()).toString();
}

void AppConfig::setValue(const QString &key, const QString &value)
{
    if (!AppConfig::appConfigKeys().contains(key))
        return;
    if (AppConfig::value(key) == value)
        return;

    if (value.isEmpty())
        get()->appConfig->remove(key);
    else
        get()->appConfig->setValue(key, value);
    get()->appConfig->sync();
}

bool AppConfig::exportConfig(const QString &fileName)
{
    if(fileName.isEmpty())
        return false;
    QSettings newConfig(fileName, JSON);
    for (const auto &key : get()->appConfig->allKeys())
        newConfig.setValue(key, get()->appConfig->value(key));
    newConfig.sync();
    return true;
}

void AppConfig::importConfig(const QString &fileName)
{
    if(fileName.isEmpty())
        return;
    QSettings oldConfig(fileName, JSON);
    for (const auto &key : oldConfig.allKeys()) {
        if (key != CONFIG::ACCESSKEY || key != CONFIG::SECRETKEY || key != CONFIG::APIURL)
            AppConfig::setValue(key, QString());
        else
            AppConfig::setValue(key, oldConfig.value(key).toString());
    }
}

QString AppConfig::operationName()
{
    return get()->appSettings->value(_opNameSetting).toString();
}

QString AppConfig::operationSlug()
{
    return get()->appSettings->value(_opSlugSetting).toString();
}

void AppConfig::setOperationDetails(const QString &operationSlug, const QString &operationName)
{
    get()->appSettings->setValue(_opNameSetting, operationName);
    get()->appSettings->setValue(_opSlugSetting, operationSlug);
    Q_EMIT get()->operationChanged(operationSlug, operationName);
}

void AppConfig::setLastUsedTags(QList<model::Tag> lastTags)
{
    QVariantList writeTags;
    for (const auto &tag : lastTags)
        writeTags << QVariant::fromValue(tag);
    get()->appSettings->setValue(_lastUsedTagsSetting, QVariant::fromValue(writeTags));
}

QList<model::Tag> AppConfig::getLastUsedTags()
{
    QList<model::Tag> rtn;
    const auto tags = get()->appSettings->value(_lastUsedTagsSetting).toList();
    for (const auto &tag : tags)
        rtn.append(tag.value<model::Tag>());
    return rtn;
}

bool AppConfig::readJSONSettings(QIODevice &device, QMap<QString, QVariant> &map)
{
    QJsonParseError error;
    QJsonObject json = QJsonDocument::fromJson(device.readAll(), &error).object();
    if(error.error != QJsonParseError::NoError) {
        qWarning() << "Error parsing settings: " << error.errorString();
        return false;
    }
    map = json.toVariantMap();
    return true;
}

bool AppConfig::writeJSONSettings(QIODevice &device, const QMap<QString, QVariant> &map)
{
    QJsonObject temp = QJsonObject::fromVariantMap(map);
    return (device.write(QJsonDocument(temp).toJson()) != -1);
}
