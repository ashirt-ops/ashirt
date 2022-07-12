// Copyright 2020, Verizon Media
// Licensed under the terms of MIT. See LICENSE file in project root for terms.

#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

void handleCLI(std::vector<std::string> args);

#ifndef QT_NO_SYSTEMTRAYICON
#include <QApplication>
#include <QMessageBox>
#include <QMetaType>

#include "appconfig.h"
#include "appsettings.h"
#include "db/databaseconnection.h"
#include "exceptions/fileerror.h"
#include "traymanager.h"

QIcon getWindowIcon();

int main(int argc, char* argv[]) {
  Q_INIT_RESOURCE(res_icons);
  Q_INIT_RESOURCE(res_migrations);

  QCoreApplication::setApplicationName("ashirt");

#ifdef Q_OS_WIN
  QCoreApplication::setOrganizationName("ashirt");
#endif


  DatabaseConnection* conn = new DatabaseConnection(Constants::dbLocation, Constants::defaultDbName);
  if(!conn->connect()) {
    QTextStream(stderr) << "Unable to connect to Database" << Qt::endl;
    return -1;
  }

  auto configError = AppConfig::getInstance().errorText;
  if (!configError.isEmpty()) {  // quick check & preload config data
    QTextStream(stderr) << "Unable to load config file: " << configError << Qt::endl;
    return -1;
  }

  int rtn;
  try {
    QApplication app(argc, argv);
    qRegisterMetaType<model::Tag>();

    app.setWindowIcon(getWindowIcon());

    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
      handleCLI(std::vector<std::string>(argv, argv + argc));
    }
    QApplication::setQuitOnLastWindowClosed(false);

    QObject::connect(&app, &QApplication::aboutToQuit, [conn] {
        conn->close();
        delete conn;
    });

    auto window = new TrayManager(nullptr, conn);
    rtn = app.exec();
    AppSettings::getInstance().sync();
    window->deleteLater();
  }
  catch (std::exception const& ex) {
    QTextStream(stderr) << "Exception while running: " << ex.what() << Qt::endl;
  }
  catch (...) {
    QTextStream(stderr) << "Unhandled exception while running" << Qt::endl;
  }
  return rtn;
}

QIcon getWindowIcon() {
  return QIcon(QStringLiteral(":icons/windowIcon.png"));
}

#else

#include <QDebug>
#include <QLabel>

int main(int argc, char *argv[]) { handleCLI(std::vector<string>(argv, argv + argc)); }

#endif

void handleCLI(std::vector<std::string> args) {
  size_t trueCount = args.size() - 1;
  QTextStream(stdout) << "You provided " << trueCount << " arguments.\n";
  if (trueCount == 0) {
    QTextStream(stdout) << "Next time try suppling some arguments." << Qt::endl;
    return;
  }

  QTextStream(stdout) << "All arguments:" << Qt::endl;
  for (size_t i = 1; i < args.size(); i++) {
    QTextStream(stdout) << "\t" << QString::fromStdString(args.at(i)) << Qt::endl;
  }
}
