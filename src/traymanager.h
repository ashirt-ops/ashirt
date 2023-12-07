#pragma once

#include <QActionGroup>
#include <QSystemTrayIcon>

#include "db/databaseconnection.h"
#include "dtos/operation.h"
#include "dtos/github_release.h"
#include "forms/credits/credits.h"
#include "forms/evidence/evidencemanager.h"
#include "forms/porting/porting_dialog.h"
#include "forms/settings/settings.h"
#include "helpers/screenshot.h"
#include "forms/add_operation/createoperation.h"

#ifndef QT_NO_SYSTEMTRAYICON

#include <QDialog>

QT_BEGIN_NAMESPACE
class QAction;
class QCheckBox;
class QComboBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QMenu;
class QPushButton;
class QSpinBox;
class QTextEdit;
class QTimer;
QT_END_NAMESPACE

/**
 * @brief The MessageType enum specifies how to respond to a click on a tray message
 * @see openServicesPath
 */
enum MessageType {
  /// NO_ACTION indicates that nothing should happen on a tray message click
  NO_ACTION,
  /// UPGRADE indicates that the user should be taken to the releases page
  UPGRADE,
  /// OPEN_PATH indicates that the user should be taken to a file browser
  OPEN_PATH
};

class TrayManager : public QDialog {
  Q_OBJECT

 public:
  TrayManager(QWidget* parent = nullptr, DatabaseConnection *db = nullptr);
  ~TrayManager();

 private:
  void buildUi();
  void wireUi();
  qint64 createNewEvidence(const QString& filepath, const QString& evidenceType);
  void spawnGetInfoWindow(qint64 evidenceID);
  void showNoOperationSetTrayMessage();
  void showDBWriteErrorTrayMessage();
  void checkForUpdate();
  void cleanChooseOpSubmenu();
  /// setTrayMessage mostly mirrors QSystemTrayIcon::showMessage, but adds the ability to set a message type,
  /// providing a mechanism to smartly route the click to an action.
  void setTrayMessage(MessageType type, const QString& title, const QString& message,
                      QSystemTrayIcon::MessageIcon icon=QSystemTrayIcon::Information, int millisecondsTimeoutHint = 10000);
  QIcon getTrayIcon();

 private slots:
  void onOperationListUpdated(bool success, const QList<dto::Operation> &operations);
  void onReleaseCheck(bool success, const QList<dto::GithubRelease>& releases);
  void onTrayMessageClicked();

 public slots:
  void onScreenshotCaptured(const QString &filepath);
  void setActiveOperationLabel();
  void onClipboardCapture();
  void captureAreaActionTriggered();
  void captureWindowActionTriggered();
  void captureClipboardActionTriggered();

 protected:
  void closeEvent(QCloseEvent *event) override;
  void changeEvent(QEvent *event) override;

 private:
  inline static const int MS_IN_DAY = 86400000;
  QString _recordErrorTitle = tr("Unable to Record Evidence");
  DatabaseConnection *db = nullptr;
  Screenshot *screenshotTool = nullptr;
  QTimer *updateCheckTimer = nullptr;
  MessageType currentTrayMessage = NO_ACTION;

  ///Show the Settings Wizard
  void showWizard();

  ///Show the Welcome Screen
  void showWelcomeScreen();

  /// openServicesPath is a variable to store where, on click, to open a path the next time a tray message is displayed
  QString openServicesPath;

  // Subwindows
  Settings *settingsWindow = nullptr;
  EvidenceManager *evidenceManagerWindow = nullptr;
  Credits *creditsWindow = nullptr;
  PortingDialog *importWindow = nullptr;
  PortingDialog *exportWindow = nullptr;
  CreateOperation *createOperationWindow = nullptr;

  // UI Elements
  QSystemTrayIcon *trayIcon = nullptr;
  QMenu *chooseOpSubmenu = nullptr;
  QAction *newOperationAction = nullptr;
  QAction *selectedAction = nullptr;  // note: do not delete; for reference only
  QActionGroup allOperationActions;
};
#endif  // QT_NO_SYSTEMTRAYICON
