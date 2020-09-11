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
  hotkeyManager = new HotkeyManager();
  hotkeyManager->updateHotkeys();

  settingsWindow = new Settings(hotkeyManager, this);
  evidenceManagerWindow = new EvidenceManager(db, this);
  creditsWindow = new Credits(this);

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
  wireUi();

  // delayed so that windows can listen for get all ops signal
  NetMan::getInstance().refreshOperationsList();
  // this should probably be made dynamic, to support forking
  NetMan::getInstance().checkForNewRelease("google", "go-github");
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
  // delete all of the existing events
  for (QAction* act : allOperationActions) {
    chooseOpSubmenu->removeAction(act);
    delete act;
  }
  allOperationActions.clear();
  selectedAction = nullptr; // clear the selected action to ensure no funny business
}

void TrayManager::wireUi() {
  connect(screenshotTool, &Screenshot::onScreenshotCaptured, this,
          &TrayManager::onScreenshotCaptured);

  connect(hotkeyManager, &HotkeyManager::codeblockHotkeyPressed, this,
          &TrayManager::captureCodeblockActionTriggered);
  connect(hotkeyManager, &HotkeyManager::captureAreaHotkeyPressed, this,
          &TrayManager::captureAreaActionTriggered);
  connect(hotkeyManager, &HotkeyManager::captureWindowHotkeyPressed, this,
          &TrayManager::captureWindowActionTriggered);

  connect(&NetMan::getInstance(), &NetMan::operationListUpdated, this,
          &TrayManager::onOperationListUpdated);
  connect(&NetMan::getInstance(), &NetMan::releasesChecked, this, &TrayManager::onReleaseCheck);
  connect(&AppSettings::getInstance(), &AppSettings::onOperationUpdated, this,
          &TrayManager::setActiveOperationLabel);
  connect(trayIcon, &QSystemTrayIcon::messageClicked, creditsWindow, &QWidget::show);
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
  connect(captureScreenAreaAction, &QAction::triggered, this, &TrayManager::captureAreaActionTriggered);

  captureWindowAction = new QAction(tr("Capture Window"), this);
  connect(captureWindowAction, &QAction::triggered, this, &TrayManager::captureWindowActionTriggered);

  showEvidenceManagerAction = new QAction(tr("View Accumulated Evidence"), this);
  connect(showEvidenceManagerAction, &QAction::triggered, evidenceManagerWindow, &QWidget::show);

  showCreditsAction = new QAction(tr("About"), this);
  connect(showCreditsAction, &QAction::triggered, creditsWindow, &QWidget::show);

  addCodeblockAction = new QAction(tr("Add Codeblock from Clipboard"), this);
  connect(addCodeblockAction, &QAction::triggered, this, &TrayManager::captureCodeblockActionTriggered);

  chooseOpSubmenu = new QMenu(tr("Select Operation"));
  chooseOpStatusAction = new QAction("Loading operations...", chooseOpSubmenu);
  chooseOpStatusAction->setEnabled(false);

  chooseOpSubmenu->addAction(chooseOpStatusAction);
  chooseOpSubmenu->addSeparator();
}

void TrayManager::spawnGetInfoWindow(qint64 evidenceID) {
  auto getInfoWindow = new GetInfo(db, evidenceID, this);
  connect(getInfoWindow, &GetInfo::evidenceSubmitted, [](model::Evidence evi){
    AppSettings::getInstance().setLastUsedTags(evi.tags);
  });
  getInfoWindow->show();
}

qint64 TrayManager::createNewEvidence(QString filepath, QString evidenceType) {
  AppSettings& inst = AppSettings::getInstance();
  auto evidenceID = db->createEvidence(filepath, inst.operationSlug(), evidenceType);
  auto tags = inst.getLastUsedTags();
  if (tags.size() > 0) {
    db->setEvidenceTags(tags, evidenceID);
  }
  return evidenceID;
}

void TrayManager::captureWindowActionTriggered() {
  if(AppSettings::getInstance().operationSlug() == "") {
    showNoOperationSetTrayMessage();
    return;
  }
  screenshotTool->captureWindow();
}

void TrayManager::captureAreaActionTriggered() {
  if(AppSettings::getInstance().operationSlug() == "") {
    showNoOperationSetTrayMessage();
    return;
  }
  screenshotTool->captureArea();
}

void TrayManager::captureCodeblockActionTriggered() {
  if(AppSettings::getInstance().operationSlug() == "") {
    showNoOperationSetTrayMessage();
    return;
  }
  onCodeblockCapture();
}

void TrayManager::onCodeblockCapture() {
  QString clipboardContent = ClipboardHelper::readPlaintext();
  if (clipboardContent != "") {
    Codeblock evidence(clipboardContent);
    Codeblock::saveCodeblock(evidence);
    try {
      auto evidenceID = createNewEvidence(evidence.filePath(), "codeblock");
      spawnGetInfoWindow(evidenceID);
    }
    catch (QSqlError& e) {
      std::cout << "could not write to the database: " << e.text().toStdString() << std::endl;
    }
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
  connect(trayIcon, &QSystemTrayIcon::activated, [this]{
    chooseOpStatusAction->setText("Loading operations...");
    NetMan::getInstance().refreshOperationsList();
  });
}

void TrayManager::onScreenshotCaptured(const QString& path) {
  try {
    auto evidenceID = createNewEvidence(path, "image");
    spawnGetInfoWindow(evidenceID);
  }
  catch (QSqlError& e) {
    std::cout << "could not write to the database: " << e.text().toStdString() << std::endl;
  }
}

void TrayManager::showNoOperationSetTrayMessage() {
  trayIcon->showMessage("Unable to Record Evidence",
                        "No Operation has been selected. Please select an operation first.",
                        QSystemTrayIcon::Warning);
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
        AppSettings::getInstance().setLastUsedTags(std::vector<model::Tag>{}); // clear last used tags
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
    if (selectedAction == nullptr) {
      AppSettings::getInstance().setOperationDetails("", "");
    }
  }
  else {
    chooseOpStatusAction->setText(tr("Unable to load operations"));
  }
}

void TrayManager::onReleaseCheck(bool success, std::vector<dto::GithubRelease> releases) {
  if (!success) {
    return;  // doesn't matter if this fails
  }

  auto digest = dto::ReleaseDigest::fromReleases("v29.0.0", releases); // TODO: replace with proper verion

  if (digest.hasUpgrade()) {
    this->trayIcon->showMessage("Updates are available!", "Click for more info");
  }
}

#endif
