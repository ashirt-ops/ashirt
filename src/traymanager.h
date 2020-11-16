// Copyright 2020, Verizon Media
// Licensed under the terms of MIT. See LICENSE file in project root for terms.

#ifndef WINDOW_H
#define WINDOW_H

#include <QSystemTrayIcon>
#include <QTimer>

#include "db/databaseconnection.h"
#include "dtos/operation.h"
#include "dtos/github_release.h"
#include "forms/credits/credits.h"
#include "forms/evidence/evidencemanager.h"
#include "forms/porting/porting_dialog.h"
#include "forms/settings/settings.h"
#include "helpers/screenshot.h"
#include "hotkeymanager.h"
#include "tools/UGlobalHotkey/uglobalhotkeys.h"
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
QT_END_NAMESPACE

enum MessageType {
  NO_ACTION,
  UPGRADE,
};

class TrayManager : public QDialog {
  Q_OBJECT

 public:
  TrayManager(DatabaseConnection *);
  ~TrayManager();

 private:
  void buildUi();
  void wireUi();
  qint64 createNewEvidence(QString filepath, QString evidenceType);
  void spawnGetInfoWindow(qint64 evidenceID);
  void showNoOperationSetTrayMessage();
  void checkForUpdate();
  void cleanChooseOpSubmenu();
  void setTrayMessage(MessageType type, QString title, QString message,
                      QSystemTrayIcon::MessageIcon icon=QSystemTrayIcon::Information, int millisecondsTimeoutHint = 10000);

 private slots:
  void onOperationListUpdated(bool success, const std::vector<dto::Operation> &operations);
  void onReleaseCheck(bool success, std::vector<dto::GithubRelease> releases);
  void onTrayMessageClicked();

 public slots:
  void onScreenshotCaptured(const QString &filepath);
  void setActiveOperationLabel();
  void onCodeblockCapture();
  void captureAreaActionTriggered();
  void captureWindowActionTriggered();
  void captureCodeblockActionTriggered();

 protected:
  void closeEvent(QCloseEvent *event) override;

 private:
  DatabaseConnection *db = nullptr;
  HotkeyManager *hotkeyManager = nullptr;
  Screenshot *screenshotTool = nullptr;
  QTimer *updateCheckTimer = nullptr;
  MessageType currentTrayMessage = NO_ACTION;

  // Subwindows
  Settings *settingsWindow = nullptr;
  EvidenceManager *evidenceManagerWindow = nullptr;
  Credits *creditsWindow = nullptr;
  PortingDialog *importWindow = nullptr;
  PortingDialog *exportWindow = nullptr;
  CreateOperation *createOperationWindow = nullptr;

  // UI Elements
  QSystemTrayIcon *trayIcon = nullptr;
  QMenu *trayIconMenu = nullptr;

  QAction *quitAction = nullptr;
  QAction *currentOperationMenuAction = nullptr;
  QAction *captureScreenAreaAction = nullptr;
  QAction *captureWindowAction = nullptr;
  QAction *showEvidenceManagerAction = nullptr;
  QAction *showCreditsAction = nullptr;
  QAction *addCodeblockAction = nullptr;

  QMenu *settingsSubmenu = nullptr;
  QAction *exportAction = nullptr;
  QAction *importAction = nullptr;
  QAction *showSettingsAction = nullptr;

  QMenu *chooseOpSubmenu = nullptr;
  QAction *chooseOpStatusAction = nullptr;
  QAction *newOperationAction = nullptr;
  QAction *selectedAction = nullptr;  // note: do not delete; for reference only
  std::vector<QAction *> allOperationActions;
};

#endif  // QT_NO_SYSTEMTRAYICON

#endif
