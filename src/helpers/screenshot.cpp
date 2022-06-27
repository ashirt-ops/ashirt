// Copyright 2020, Verizon Media
// Licensed under the terms of MIT. See LICENSE file in project root for terms.

#include "screenshot.h"

#include <QDir>
#include <QFile>
#include <QObject>
#include <QProcess>

#include "appconfig.h"
#include "helpers/file_helpers.h"
#include "helpers/system_helpers.h"

Screenshot::Screenshot(QObject *parent) : QObject(parent) {}

void Screenshot::captureArea() { basicScreenshot(AppConfig::getInstance().screenshotExec); }

void Screenshot::captureWindow() { basicScreenshot(AppConfig::getInstance().captureWindowExec); }

QString Screenshot::mkName() {
  return FileHelpers::randomFilename(QStringLiteral("ashirt_screenshot_XXXXXX.%1").arg(extension()));
}

void Screenshot::basicScreenshot(QString cmdProto)
{
    if(!QDir().mkpath(SystemHelpers::pathToEvidence()))
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

    connect(ssTool, &QProcess::finished, this, [this, tempFile, newName] {
        if(!QFile::exists(tempFile))
            return;
        auto finalName = QDir::toNativeSeparators(SystemHelpers::pathToEvidence().append(newName));
        auto trueName = QFile::rename(tempFile, finalName) ? finalName : newName;
        Q_EMIT onScreenshotCaptured(trueName);
    });
    connect(ssTool, &QProcess::aboutToClose, ssTool, &QProcess::deleteLater);
}
