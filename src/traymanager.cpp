// Copyright 2020, Verizon Media
// Licensed under the terms of MIT. See LICENSE file in project root for terms.

#include "traymanager.h"

#ifndef QT_NO_SYSTEMTRAYICON

#include <QAction>
#include <QApplication>
#include <QCloseEvent>
#include <QCoreApplication>
#include <QMenu>
#include <QMessageBox>
#include <QTimer>
#include <QDesktopServices>
#include <iostream>

#include "appconfig.h"
#include "appsettings.h"
#include "db/databaseconnection.h"
#include "forms/getinfo/getinfo.h"
#include "helpers/clipboard/clipboardhelper.h"
#include "helpers/netman.h"
#include "helpers/screenshot.h"
#include "helpers/constants.h"
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
  updateCheckTimer = new QTimer(this);
  updateCheckTimer->start(24*60*60*1000); // every day

  buildUi();
  wireUi();

  // delayed so that windows can listen for get all ops signal
  NetMan::getInstance().refreshOperationsList();
  QTimer::singleShot(5000, this, &TrayManager::checkForUpdate);
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

  delete updateCheckTimer;
  delete trayIconMenu;
  delete trayIcon;

  delete screenshotTool;
  delete hotkeyManager;
  delete settingsWindow;
  delete evidenceManagerWindow;
  delete creditsWindow;
}

void TrayManager::buildUi() {
  // create subwindows
  settingsWindow = new Settings(hotkeyManager, this);
  evidenceManagerWindow = new EvidenceManager(db, this);
  creditsWindow = new Credits(this);
  createOperationWindow = new CreateOperation(this);

  trayIconMenu = new QMenu(this);
  chooseOpSubmenu = new QMenu(tr("Select Operation"));

  // small helper to create an action and assign it to the tray
  auto addToTray = [this](QString text, QAction** act){
    *act = new QAction(text, this);
    trayIconMenu->addAction(*act);
  };

  // Tray Ordering
  addToTray(tr("Add Codeblock from Clipboard"), &addCodeblockAction);
  addToTray(tr("Capture Screen Area"), &captureScreenAreaAction);
  addToTray(tr("Capture Window"), &captureWindowAction);
  addToTray(tr("View Accumulated Evidence"), &showEvidenceManagerAction);
  addToTray(tr("Settings"), &showSettingsAction);
  trayIconMenu->addSeparator();
  addToTray(tr(""), &currentOperationMenuAction);
  trayIconMenu->addMenu(chooseOpSubmenu);
  trayIconMenu->addSeparator();
  addToTray(tr("About"), &showCreditsAction);
  addToTray(tr("Quit"), &quitAction);

  // finish action config
  currentOperationMenuAction->setEnabled(false);
  chooseOpStatusAction = new QAction("Loading operations...", chooseOpSubmenu);
  chooseOpStatusAction->setEnabled(false);
  newOperationAction = new QAction("New Operation", chooseOpSubmenu);
  newOperationAction->setEnabled(false); // only enable when we have an internet connection
  chooseOpSubmenu->addAction(chooseOpStatusAction);
  chooseOpSubmenu->addAction(newOperationAction);
  chooseOpSubmenu->addSeparator();

  setActiveOperationLabel();

  QIcon icon = QIcon(ICON);
  // TODO: figure out if any other environments support masking
#ifdef Q_OS_MACOS
  icon.setIsMask(true);
#endif

  trayIcon = new QSystemTrayIcon(this);
  trayIcon->setContextMenu(trayIconMenu);
  trayIcon->setIcon(icon);
  trayIcon->show();
}

void TrayManager::wireUi() {
  auto toTop = [](QDialog* window) {
    window->show(); // display the window
    window->raise(); // bring to the top (mac)
    window->activateWindow(); // alternate bring to the top (windows)
  };
  auto actTriggered = &QAction::triggered;
  // connect actions
  connect(quitAction, actTriggered, qApp, &QCoreApplication::quit);
  connect(showSettingsAction, actTriggered, [this, toTop](){toTop(settingsWindow);});
  connect(captureScreenAreaAction, actTriggered, this, &TrayManager::captureAreaActionTriggered);
  connect(captureWindowAction, actTriggered, this, &TrayManager::captureWindowActionTriggered);
  connect(showEvidenceManagerAction, actTriggered, [this, toTop](){toTop(evidenceManagerWindow);});
  connect(showCreditsAction, actTriggered, [this, toTop](){toTop(creditsWindow);});
  connect(addCodeblockAction, actTriggered, this, &TrayManager::captureCodeblockActionTriggered);
  connect(newOperationAction, actTriggered, [this, toTop](){toTop(createOperationWindow);});

  connect(screenshotTool, &Screenshot::onScreenshotCaptured, this,
          &TrayManager::onScreenshotCaptured);

  // connect to hotkey signals
  connect(hotkeyManager, &HotkeyManager::codeblockHotkeyPressed, this,
          &TrayManager::captureCodeblockActionTriggered);
  connect(hotkeyManager, &HotkeyManager::captureAreaHotkeyPressed, this,
          &TrayManager::captureAreaActionTriggered);
  connect(hotkeyManager, &HotkeyManager::captureWindowHotkeyPressed, this,
          &TrayManager::captureWindowActionTriggered);

  // connect to network signals
  connect(&NetMan::getInstance(), &NetMan::operationListUpdated, this,
          &TrayManager::onOperationListUpdated);
  connect(&NetMan::getInstance(), &NetMan::releasesChecked, this, &TrayManager::onReleaseCheck);
  connect(&AppSettings::getInstance(), &AppSettings::onOperationUpdated, this,
          &TrayManager::setActiveOperationLabel);
  
  connect(trayIcon, &QSystemTrayIcon::messageClicked, [](){QDesktopServices::openUrl(Constants::releasePageUrl());});
  connect(trayIcon, &QSystemTrayIcon::activated, [this] {
    chooseOpStatusAction->setText("Loading operations...");
    newOperationAction->setEnabled(false);
    NetMan::getInstance().refreshOperationsList();
  });

  connect(updateCheckTimer, &QTimer::timeout, this, &TrayManager::checkForUpdate);
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
    newOperationAction->setEnabled(true);
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

void TrayManager::checkForUpdate() {
  NetMan::getInstance().checkForNewRelease(Constants::releaseOwner(), Constants::releaseRepo());
}

void TrayManager::onReleaseCheck(bool success, std::vector<dto::GithubRelease> releases) {
  if (!success) {
    return;  // doesn't matter if this fails -- another request will be made later.
  }

  auto digest = dto::ReleaseDigest::fromReleases(Constants::releaseTag(), releases);

  if (digest.hasUpgrade()) {
    this->trayIcon->showMessage("A new version is available!", "Click for more info");
  }
}

#endif
