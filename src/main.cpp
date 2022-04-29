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
#include "exceptions/databaseerr.h"
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

  DatabaseConnection* conn;
  try {
    conn = new DatabaseConnection(Constants::dbLocation(), Constants::defaultDbName());
    conn->connect();
  }
  catch (FileError& err) {
    std::cout << err.what() << std::endl;
    return -1;
  }
  catch (DBDriverUnavailableError& err) {
    std::cout << err.what() << std::endl;
    return -1;
  }
  catch (QSqlError& e) {
    std::cout << e.text().toStdString() << std::endl;
    return -1;
  }
  catch (std::exception& e) {
    std::cout << "Unexpected error: " << e.what() << std::endl;
    return -1;
  }

  auto configError = AppConfig::getInstance().errorText.toStdString();
  if (!configError.empty()) {  // quick check & preload config data
    std::cout << "Unable to load config file: " << configError << std::endl;
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

    auto window = new TrayManager(conn);
    rtn = app.exec();
    AppSettings::getInstance().sync();
    delete window;
  }
  catch (std::exception const& ex) {
    std::cout << "Exception while running: " << ex.what() << std::endl;
  }
  catch (...) {
    std::cout << "Unhandled exception while running" << std::endl;
  }
  conn->close();
  delete conn;

  return rtn;
}

QIcon getWindowIcon() {
  return QIcon(":icons/windowIcon.png");
}

#else

#include <QDebug>
#include <QLabel>

int main(int argc, char *argv[]) { handleCLI(std::vector<string>(argv, argv + argc)); }

#endif

void handleCLI(std::vector<std::string> args) {
  size_t trueCount = args.size() - 1;
  std::cout << "You provided " << trueCount << " arguments.\n";
  if (trueCount == 0) {
    std::cout << "Next time try suppling some arguments." << std::endl;
    return;
  }

  std::cout << "All arguments:" << std::endl;
  for (size_t i = 1; i < args.size(); i++) {
    std::cout << "\t" << args.at(i) << std::endl;
  }
}
