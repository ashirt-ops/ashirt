// Copyright 2020, Verizon Media
// Licensed under the terms of GPLv3. See LICENSE file in project root for terms.

#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

void handleCLI(std::vector<std::string> args);

#ifndef QT_NO_SYSTEMTRAYICON
#include <QApplication>
#include <QMessageBox>

#include "appconfig.h"
#include "appsettings.h"
#include "db/databaseconnection.h"
#include "exceptions/databaseerr.h"
#include "exceptions/fileerror.h"
#include "traymanager.h"

QDataStream& operator<<(QDataStream& out, const model::Tag& v) {
  out << v.tagName << v.id << v.serverTagId;
  return out;
}

QDataStream& operator>>(QDataStream& in, model::Tag& v) {
  in >> v.tagName;
  in >> v.id;
  in >> v.serverTagId;
  return in;
}

QDataStream& operator<<(QDataStream& out, const std::vector<model::Tag>& v) {
  out << int(v.size());
  for (auto tag : v) {
    out << tag;
  }
  return out;
}

QDataStream& operator>>(QDataStream& in, std::vector<model::Tag>& v) {
  int qty;
  in >> qty;
  v.reserve(qty);
  for(int i = 0; i < qty; i++) {
    model::Tag t;
    in >> t;
    v.push_back(t);
  }
  return in;
}

int main(int argc, char* argv[]) {
  Q_INIT_RESOURCE(res_icons);
  Q_INIT_RESOURCE(res_migrations);

  QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
  QCoreApplication::setOrganizationName("Verizon Media Group");
  QCoreApplication::setOrganizationDomain("verizon.com");
  QCoreApplication::setApplicationName("AShirt Screenshot");

  DatabaseConnection* conn;
  try {
    conn = new DatabaseConnection();
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
    qRegisterMetaTypeStreamOperators<model::Tag>("Tag");
    qRegisterMetaTypeStreamOperators<std::vector<model::Tag>>("TagVector");
    QApplication app(argc, argv);

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
