// Copyright 2020, Verizon Media
// Licensed under the terms of MIT. See LICENSE file in project root for terms.

#ifndef WINDOW_H
#define WINDOW_H

#include <QSystemTrayIcon>

#include "db/databaseconnection.h"
#include "dtos/operation.h"
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

 private slots:
  void onOperationListUpdated(bool success, const std::vector<dto::Operation> &operations);

 private:
  void createActions();
  void createTrayMenu();
  void wireUi();
  qint64 createNewEvidence(QString filepath, QString evidenceType);
  void spawnGetInfoWindow(qint64 evidenceID);

 private:
  QAction *quitAction;
  QAction *showSettingsAction;
  QAction *currentOperationMenuAction;
  QAction *captureScreenAreaAction;
  QAction *captureWindowAction;
  QAction *showEvidenceManagerAction;
  QAction *showCreditsAction;
  QAction *addCodeblockAction;

  void cleanChooseOpSubmenu();
  QMenu *chooseOpSubmenu;
  QAction *chooseOpStatusAction;
  QAction *refreshOperationListAction;
  QAction *selectedAction = nullptr;  // note: do not delete; for reference only
  std::vector<QAction *> allOperationActions;

  QSystemTrayIcon *trayIcon;
  QMenu *trayIconMenu;

  Settings *settingsWindow;
  EvidenceManager *evidenceManagerWindow;
  Credits *creditsWindow;

  Screenshot *screenshotTool;
  HotkeyManager *hotkeyManager;

  DatabaseConnection *db;
};

#endif  // QT_NO_SYSTEMTRAYICON

#endif
