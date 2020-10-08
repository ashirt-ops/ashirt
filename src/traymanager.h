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
#include "forms/settings/settings.h"
#include "helpers/screenshot.h"
#include "hotkeymanager.h"
#include "tools/UGlobalHotkey/uglobalhotkeys.h"

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

class TrayManager : public QDialog {
  Q_OBJECT

 public:
  TrayManager(DatabaseConnection *);
  ~TrayManager();

 protected:
  void closeEvent(QCloseEvent *event) override;

 public slots:
  void onScreenshotCaptured(const QString &filepath);
  void setActiveOperationLabel();
  void onCodeblockCapture();
  void captureAreaActionTriggered();
  void captureWindowActionTriggered();
  void captureCodeblockActionTriggered();

 private slots:
  void onOperationListUpdated(bool success, const std::vector<dto::Operation> &operations);
  void onReleaseCheck(bool success, std::vector<dto::GithubRelease> releases);

 private:
  void createActions();
  void createTrayMenu();
  void wireUi();
  qint64 createNewEvidence(QString filepath, QString evidenceType);
  void spawnGetInfoWindow(qint64 evidenceID);
  void showNoOperationSetTrayMessage();
  void checkForUpdate();

 private:
  QAction *quitAction = nullptr;
  QAction *showSettingsAction = nullptr;
  QAction *currentOperationMenuAction = nullptr;
  QAction *captureScreenAreaAction = nullptr;
  QAction *captureWindowAction = nullptr;
  QAction *showEvidenceManagerAction = nullptr;
  QAction *showCreditsAction = nullptr;
  QAction *addCodeblockAction = nullptr;

  void cleanChooseOpSubmenu();
  QMenu *chooseOpSubmenu = nullptr;
  QAction *chooseOpStatusAction = nullptr;
  QAction *selectedAction = nullptr;  // note: do not delete; for reference only
  std::vector<QAction *> allOperationActions;

  QSystemTrayIcon *trayIcon = nullptr;
  QMenu *trayIconMenu = nullptr;

  Settings *settingsWindow = nullptr;
  EvidenceManager *evidenceManagerWindow = nullptr;
  Credits *creditsWindow = nullptr;

  Screenshot *screenshotTool = nullptr;
  HotkeyManager *hotkeyManager = nullptr;

  DatabaseConnection *db = nullptr;
  QTimer *updateCheckTimer = nullptr;
};

#endif  // QT_NO_SYSTEMTRAYICON

#endif
