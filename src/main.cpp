#include <QApplication>
#include <QMessageBox>
#include <QMetaType>

#include "db/databaseconnection.h"
#include "traymanager.h"

QIcon getWindowIcon() { return QIcon(QStringLiteral(":icons/windowIcon.png")); }

void showMsgBox(const QString &errorText = QString())
{
    QMessageBox::critical(nullptr, QT_TRANSLATE_NOOP("main", "ASHIRT Error"), errorText);
}

int main(int argc, char* argv[])
{
    Q_INIT_RESOURCE(res_icons);
    Q_INIT_RESOURCE(res_migrations);

    QCoreApplication::setApplicationName(QStringLiteral("ashirt"));
#ifdef Q_OS_WIN
    QCoreApplication::setOrganizationName(QCoreApplication::applicationName());

#endif

    QApplication app(argc, argv);
    app.setWindowIcon(getWindowIcon());
#ifdef Q_OS_WIN
    app.setStyle("fusion");
#endif

    if(!QSystemTrayIcon::isSystemTrayAvailable()) {
        showMsgBox(QT_TRANSLATE_NOOP("main", "A System tray is required to interact with the application"));
        return -1;
    }

    auto conn = new DatabaseConnection(Constants::dbLocation, Constants::defaultDbName);
    if(!conn->connect()) {
        showMsgBox(QString(QT_TRANSLATE_NOOP("main", "Database Error: %1")).arg(conn->errorString()));
        return -1;
    }

    app.setQuitOnLastWindowClosed(false);
    qRegisterMetaType<model::Tag>();
    auto window = new TrayManager(nullptr, conn);

    QObject::connect(&app, &QApplication::aboutToQuit, [conn] {
        conn->close();
        delete conn;
    });

    int rtn = app.exec();
    window->deleteLater();
    return rtn;
}
