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
  if (!conn->connect()) {
    QMessageBox::critical(nullptr, QStringLiteral("ASHIRT Error"), QStringLiteral("Unable to connect to database"));
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
    window->deleteLater();
  }
  catch (std::exception const& ex) {
    qWarning() << "Exception while running: " << ex.what();
  }
  catch (...) {
    qWarning() << "Unhandled exception while running";
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
  qInfo() << "You provided " << trueCount << " arguments";
  if (trueCount == 0) {
    qInfo() << "Next time try suppling some arguments.";
    return;
  }

  qInfo() << "All arguments:";
  for (size_t i = 1; i < args.size(); i++) {
    qInfo() << "\t" << QString::fromStdString(args.at(i));
  }
}
