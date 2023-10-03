// Copyright 2020, Verizon Media
// Licensed under the terms of MIT. See LICENSE file in project root for terms.

#include "screenshot.h"

#include <QDir>
#include <QFile>
#include <QObject>
#include <QProcess>

#include "appconfig.h"
#include "helpers/string_helpers.h"
#include "helpers/system_helpers.h"

#ifndef Q_OS_WIN
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#endif

Screenshot::Screenshot(QObject *parent) : QObject(parent) {}

void Screenshot::captureArea() { basicScreenshot(AppConfig::value(CONFIG::COMMAND_SCREENSHOT)); }

void Screenshot::captureWindow() {
  auto captureCommand = AppConfig::value(CONFIG::COMMAND_CAPTUREWINDOW);
  if(!captureCommand.startsWith("DBUS")) {
    basicScreenshot(AppConfig::value(CONFIG::COMMAND_CAPTUREWINDOW));
    return;
  }

#ifndef Q_OS_WIN
  // Attempt Dbus Capture
  QDBusConnection bus = QDBusConnection::sessionBus();

  QDBusInterface *i = new QDBusInterface("org.freedesktop.portal.Desktop", "/org/freedesktop/portal/desktop", "org.freedesktop.portal.Screenshot", bus, NULL);

  QVariantMap options;
  options["interactive"] = captureCommand.endsWith(QStringLiteral("INTERACTIVE")) ? true : false;

  QDBusReply<QDBusObjectPath> repl = i->call("Screenshot", "", options);
  if(repl.isValid()) {
    bus.connect("", repl.value().path(), "org.freedesktop.portal.Request", "Response", this, SLOT(dbusScreenShot(uint, QVariantMap)));
  } else {
    qDebug() << "Something is wrong: " << repl.error();
  }
#endif
}

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

void Screenshot::dbusScreenShot(uint responseCode, QVariantMap results)
{
    if(responseCode != 0) {
        qDebug() << "Unable to take a screenshot";
        return;
    }
    auto baseDir = SystemHelpers::pathToEvidence();
    if(!QDir().mkpath(baseDir))
        return;
    auto newName = mkName();
    auto tempFile = QDir::toNativeSeparators(m_fileTemplate.arg(QDir::tempPath(), newName));
    auto oldFile = results["uri"].toString().mid(7);
    QFile::copy(oldFile, tempFile);
    QFile::remove(oldFile);
    Q_EMIT onScreenshotCaptured(tempFile);
}
