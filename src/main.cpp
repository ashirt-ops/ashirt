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
#include <QScreen>

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
  QCoreApplication::setApplicationName("ashirt");

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

    auto screens = app.screens();
    auto ps = app.primaryScreen();
    int i = 0;
    auto printRect = [](QRect r){
      auto rtn = QString("QRect(%1, %2, %3, %4)").arg(r.x()).arg(r.y()).arg(r.width()).arg(r.height());
      return rtn;
    };

    for (auto s : screens) {
      i++;
      std::cout<< "details for screen: " << i << std::endl;
      std::cout << "Primary? " << (s == ps) << std::endl;
      std::cout << "name: " << s->name().toStdString() << std::endl;
      std::cout << "width: " << s->geometry().width() << "  height: " << s->geometry().height()<< std::endl;

      app.connect(s, &QScreen::availableGeometryChanged, [i, printRect](QRect geo){
        std::cout << "Screen[" << i <<"]::availableGeometryChanged("<< printRect(geo).toStdString() <<")" << std::endl;
      });
      app.connect(s, &QScreen::geometryChanged, [i, printRect](QRect geo){
        std::cout << "Screen[" << i <<"]::geometryChanged("<< printRect(geo).toStdString() <<")" << std::endl;
      });
      app.connect(s, &QScreen::logicalDotsPerInchChanged, [i](qreal dpi){
        std::cout << "Screen[" << i <<"]::logicalDotsPerInchChanged("<< dpi <<")" << std::endl;
      });
      app.connect(s, &QScreen::orientationChanged, [i, window](Qt::ScreenOrientation ori){
        window->setTrayMenu();
        std::cout << "Screen[" << i <<"]::orientationChanged("<< ori <<")" << std::endl;
      });
      app.connect(s, &QScreen::physicalDotsPerInchChanged, [i](qreal dpi){
        std::cout << "Screen[" << i <<"]::physicalDotsPerInchChanged("<< dpi <<")" << std::endl;
      });
      app.connect(s, &QScreen::physicalSizeChanged, [i](const QSizeF &size){
        std::cout << "Screen[" << i <<"]::physicalSizeChanged(???)" << std::endl;
      });
      app.connect(s, &QScreen::primaryOrientationChanged, [i](Qt::ScreenOrientation ori){
        std::cout << "Screen[" << i <<"]::primaryOrientationChanged("<< ori <<")" << std::endl;
      });
      app.connect(s, &QScreen::refreshRateChanged, [i](qreal refreshRate){
        std::cout << "Screen[" << i <<"]::refreshRateChanged("<< refreshRate <<")" << std::endl;
      });
      app.connect(s, &QScreen::virtualGeometryChanged, [i, printRect](const QRect& geo){
        std::cout << "Screen[" << i <<"]::virtualGeometryChanged("<< printRect(geo).toStdString() <<")" << std::endl;
      });

    }


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
