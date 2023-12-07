#include "screenshot.h"

#include <QDir>
#include <QFile>
#include <QObject>
#include <QProcess>

#include "appconfig.h"
#include "helpers/string_helpers.h"
#include "helpers/system_helpers.h"

Screenshot::Screenshot(QObject *parent) : QObject(parent) {}

void Screenshot::captureArea() { basicScreenshot(AppConfig::value(CONFIG::COMMAND_SCREENSHOT)); }

void Screenshot::captureWindow() { basicScreenshot(AppConfig::value(CONFIG::COMMAND_CAPTUREWINDOW)); }

QString Screenshot::mkName()
{
  return QStringLiteral("ashirt_screenshot_%1.%2").arg(StringHelpers::randomString(), extension());
}

void Screenshot::basicScreenshot(QString cmdProto)
{
    auto baseDir = SystemHelpers::pathToEvidence();
    if(!QDir().mkpath(baseDir))
        return;
    auto newName = mkName();
    auto tempFile = QDir::toNativeSeparators(m_fileTemplate.arg(QDir::tempPath(), newName));
    cmdProto.replace(QStringLiteral("%file"), tempFile);

    QString app;
    if(cmdProto.startsWith(m_doubleQuote))
        app = cmdProto.mid(0, cmdProto.indexOf(m_doubleQuote, 1) +1);
    else
        app = cmdProto.mid(0, cmdProto.indexOf(m_space));
    cmdProto.remove(app);
    cmdProto = cmdProto.simplified();

    QProcess *ssTool = new QProcess(this);
    ssTool->setProgram(app);
    ssTool->setArguments(cmdProto.split(m_space));
    ssTool->setWorkingDirectory(QDir::rootPath());
    ssTool->start();

    connect(ssTool, &QProcess::finished, this, [this, baseDir, tempFile, newName] {
        if(!QFile::exists(tempFile))
            return;
        auto finalName = QDir::toNativeSeparators(baseDir + newName);
        auto trueName = QFile::rename(tempFile, finalName) ? finalName : newName;
        Q_EMIT onScreenshotCaptured(trueName);
    });
    connect(ssTool, &QProcess::aboutToClose, ssTool, &QProcess::deleteLater);
}
