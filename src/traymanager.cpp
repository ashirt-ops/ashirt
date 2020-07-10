// Copyright 2020, Verizon Media
// Licensed under the terms of MIT. See LICENSE file in project root for terms.

#include "traymanager.h"

#ifndef QT_NO_SYSTEMTRAYICON

#include <QAction>
#include <QApplication>
#include <QCheckBox>
#include <QCloseEvent>
#include <QComboBox>
#include <QCoreApplication>
#include <QDesktopWidget>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>
#include <QSpinBox>
#include <QTextEdit>
#include <QVBoxLayout>
#include <iostream>

#include "appconfig.h"
#include "appsettings.h"
#include "db/databaseconnection.h"
#include "forms/getinfo/getinfo.h"
#include "helpers/clipboard/clipboardhelper.h"
#include "helpers/netman.h"
#include "helpers/screenshot.h"
#include "hotkeymanager.h"
#include "models/codeblock.h"
#include "tools/UGlobalHotkey/uglobalhotkeys.h"

// Tray icons are handled differently between different OS and desktop
// environments. MacOS uses a monochrome mask to render a light or dark icon
// depending on which appearance is used. Gnome does not seam to be aware of a
// light/dark theme within it's tray to be able to automatically take advantage
// of this. Desktop environments and themes in Linux don't appear to be
// consistent so we will default to a light icon given that the default bars
// appear to be dark until we know more. Who knows about windows?
#ifdef Q_OS_MACOS
#define ICON ":/icons/shirt-dark.svg"
#else
#define ICON ":/icons/shirt-light.svg"
#endif

TrayManager::TrayManager(DatabaseConnection* db) {
  this->db = db;

  screenshotTool = new Screenshot();
  hotkeyManager = new HotkeyManager(screenshotTool);
  hotkeyManager->updateHotkeys();

  settingsWindow = new Settings(hotkeyManager, this);
  evidenceManagerWindow = new EvidenceManager(db, this);
  creditsWindow = new Credits(this);

  // delayed so that windows can listen for get all ops signal
  NetMan::getInstance().refreshOperationsList();

  wireUi();

  createActions();
  createTrayMenu();
  QIcon icon = QIcon(ICON);
  // TODO: figure out if any other environments support masking
#ifdef Q_OS_MACOS
  icon.setIsMask(true);
#endif
  trayIcon->setIcon(icon);

  setActiveOperationLabel();
  trayIcon->show();
}

TrayManager::~TrayManager() {
  setVisible(false);

  delete quitAction;
  delete showSettingsAction;
  delete currentOperationMenuAction;
  delete captureScreenAreaAction;
  delete captureWindowAction;
  delete showEvidenceManagerAction;
  delete showCreditsAction;
  delete addCodeblockAction;
  cleanChooseOpSubmenu();  // must be done before deleting chooseOpSubmenu/action

  delete chooseOpStatusAction;
  delete refreshOperationListAction;
  delete chooseOpSubmenu;

  delete trayIconMenu;
  delete trayIcon;

  delete screenshotTool;
  delete hotkeyManager;
  delete settingsWindow;
  delete evidenceManagerWindow;
  delete creditsWindow;
}

void TrayManager::cleanChooseOpSubmenu() {
  for (QAction* act : allOperationActions) {
    chooseOpSubmenu->removeAction(act);
    delete act;
  }
  allOperationActions.clear();
}

void TrayManager::wireUi() {
  connect(screenshotTool, &Screenshot::onScreenshotCaptured, this,
          &TrayManager::onScreenshotCaptured);
  connect(hotkeyManager, &HotkeyManager::codeblockHotkeyPressed, this,
          &TrayManager::onCodeblockCapture);

  connect(&NetMan::getInstance(), &NetMan::operationListUpdated, this,
          &TrayManager::onOperationListUpdated);
  connect(&AppSettings::getInstance(), &AppSettings::onOperationUpdated, this,
          &TrayManager::setActiveOperationLabel);
}

void TrayManager::closeEvent(QCloseEvent* event) {
#ifdef Q_OS_MACOS
  if (!event->spontaneous() || !isVisible()) {
    return;
  }
#endif
  if (trayIcon->isVisible()) {
    hide();
    event->ignore();
  }
}

void TrayManager::createActions() {
  quitAction = new QAction(tr("Quit"), this);
  connect(quitAction, &QAction::triggered, qApp, &QCoreApplication::quit);

  showSettingsAction = new QAction(tr("Settings"), this);
  connect(showSettingsAction, &QAction::triggered, settingsWindow, &QWidget::show);

  currentOperationMenuAction = new QAction(this);
  currentOperationMenuAction->setEnabled(false);

  captureScreenAreaAction = new QAction(tr("Capture Screen Area"), this);
  connect(captureScreenAreaAction, &QAction::triggered, screenshotTool, &Screenshot::captureArea);

  captureWindowAction = new QAction(tr("Capture Window"), this);
  connect(captureWindowAction, &QAction::triggered, screenshotTool, &Screenshot::captureWindow);

  showEvidenceManagerAction = new QAction(tr("View Accumulated Evidence"), this);
  connect(showEvidenceManagerAction, &QAction::triggered, evidenceManagerWindow, &QWidget::show);

  showCreditsAction = new QAction(tr("About"), this);
  connect(showCreditsAction, &QAction::triggered, creditsWindow, &QWidget::show);

  addCodeblockAction = new QAction(tr("Add Codeblock from Clipboard"), this);
  connect(addCodeblockAction, &QAction::triggered, this, &TrayManager::onCodeblockCapture);

  chooseOpSubmenu = new QMenu(tr("Select Operation"));
  chooseOpStatusAction = new QAction("Loading operations...", chooseOpSubmenu);
  chooseOpStatusAction->setEnabled(false);

  refreshOperationListAction = new QAction(tr("Refresh Operations"), chooseOpSubmenu);
  connect(refreshOperationListAction, &QAction::triggered, [this] {
    chooseOpStatusAction->setText("Loading operations...");
    NetMan::getInstance().refreshOperationsList();
  });

  chooseOpSubmenu->addAction(chooseOpStatusAction);
  chooseOpSubmenu->addAction(refreshOperationListAction);
  chooseOpSubmenu->addSeparator();
}

void TrayManager::onCodeblockCapture() {
  QString clipboardContent = ClipboardHelper::readPlaintext();
  if (clipboardContent != "") {
    Codeblock evidence(clipboardContent);
    Codeblock::saveCodeblock(evidence);
    auto evidenceID = db->createEvidence(evidence.filePath(),
                                         AppSettings::getInstance().operationSlug(), "codeblock");
    auto getInfoWindow = new GetInfo(db, evidenceID, this);
    getInfoWindow->show();
  }
}

void TrayManager::createTrayMenu() {
  trayIconMenu = new QMenu(this);

  trayIconMenu->addAction(this->addCodeblockAction);
  trayIconMenu->addAction(this->captureScreenAreaAction);
  trayIconMenu->addAction(this->captureWindowAction);
  trayIconMenu->addAction(this->showEvidenceManagerAction);
  trayIconMenu->addAction(this->showSettingsAction);
  trayIconMenu->addSeparator();
  trayIconMenu->addAction(this->currentOperationMenuAction);
  trayIconMenu->addMenu(chooseOpSubmenu);
  trayIconMenu->addSeparator();
  trayIconMenu->addAction(this->showCreditsAction);
  trayIconMenu->addAction(this->quitAction);

  trayIcon = new QSystemTrayIcon(this);
  trayIcon->setContextMenu(trayIconMenu);
}

void TrayManager::onScreenshotCaptured(const QString& path) {
  std::cout << "Captured screenshot to file: " << path.toStdString() << std::endl;
  try {
    auto evidenceID = db->createEvidence(path, AppSettings::getInstance().operationSlug(), "image");
    auto getInfoWindow = new GetInfo(db, evidenceID, this);
    getInfoWindow->show();
  }
  catch (QSqlError& e) {
    std::cout << "could not write to the database: " << e.text().toStdString() << std::endl;
  }
}

void TrayManager::setActiveOperationLabel() {
  auto opName = AppSettings::getInstance().operationName();

  QString opLabel = tr("Operation: ");
  opLabel += (opName == "") ? tr("<None>") : opName;

  currentOperationMenuAction->setText(opLabel);
}

void TrayManager::onOperationListUpdated(bool success,
                                         const std::vector<dto::Operation>& operations) {
  auto currentOp = AppSettings::getInstance().operationSlug();

  if (success) {
    chooseOpStatusAction->setText(tr("Operations loaded"));
    cleanChooseOpSubmenu();

    for (const auto& op : operations) {
      auto newAction = new QAction(op.name, chooseOpSubmenu);

      if (currentOp == op.slug) {
        newAction->setCheckable(true);
        newAction->setChecked(true);
        selectedAction = newAction;
      }

      connect(newAction, &QAction::triggered, [this, newAction, op] {
        AppSettings::getInstance().setOperationDetails(op.slug, op.name);
        if (selectedAction != nullptr) {
          selectedAction->setChecked(false);
          selectedAction->setCheckable(false);
        }
        newAction->setCheckable(true);
        newAction->setChecked(true);
        selectedAction = newAction;
      });
      allOperationActions.push_back(newAction);
      chooseOpSubmenu->addAction(newAction);
    }
  }
  else {
    chooseOpStatusAction->setText(tr("Unable to load operations"));
  }
}

#endif
