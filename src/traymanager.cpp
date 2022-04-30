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
#include "porting/system_manifest.h"

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

  delete exportAction;
  delete importAction;

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
  delete importExportSubmenu;

  delete updateCheckTimer;
  delete trayIconMenu;
  delete trayIcon;

  delete screenshotTool;
  delete hotkeyManager;
  delete settingsWindow;
  delete evidenceManagerWindow;
  delete importWindow;
  delete exportWindow;
  delete creditsWindow;
}

void TrayManager::buildUi() {
  // create subwindows
  settingsWindow = new Settings(hotkeyManager, this);
  evidenceManagerWindow = new EvidenceManager(db, this);
  creditsWindow = new Credits(this);
  importWindow = new PortingDialog(PortingDialog::Import, db, this);
  exportWindow = new PortingDialog(PortingDialog::Export, db, this);
  createOperationWindow = new CreateOperation(this);

  trayIconMenu = new QMenu(this);

  auto addMenuToMenu = [this](const QString& text, QMenu** submenu, QMenu** parent) {
    *submenu = new QMenu(text, this);
    (*parent)->addMenu(*submenu);
  };

  // small helper to create an action and assign it to a menu
  auto addToMenu = [this](const QString& text, QAction** act, QMenu** menu) {
    *act = new QAction(text, this);
    (*menu)->addAction(*act);
  };

  // Tray menu
  addToMenu(tr("Add Codeblock from Clipboard"), &addCodeblockAction, &trayIconMenu);
  addToMenu(tr("Capture Screen Area"), &captureScreenAreaAction, &trayIconMenu);
  addToMenu(tr("Capture Window"), &captureWindowAction, &trayIconMenu);
  addToMenu(tr("View Accumulated Evidence"), &showEvidenceManagerAction, &trayIconMenu);
  trayIconMenu->addSeparator();
  addToMenu(QString(), &currentOperationMenuAction, &trayIconMenu);
  addMenuToMenu(tr("Select Operation"), &chooseOpSubmenu, &trayIconMenu);
  trayIconMenu->addSeparator();
  addMenuToMenu(tr("Import/Export"), &importExportSubmenu, &trayIconMenu);
  addToMenu(tr("Settings"), &showSettingsAction, &trayIconMenu);
  addToMenu(tr("About"), &showCreditsAction, &trayIconMenu);
  addToMenu(tr("Quit"), &quitAction, &trayIconMenu);

  // Operations Submenu
  currentOperationMenuAction->setEnabled(false);
  addToMenu(tr("Loading operations..."), &chooseOpStatusAction, &chooseOpSubmenu);
  addToMenu(tr("New Operation"), &newOperationAction, &chooseOpSubmenu);

  chooseOpStatusAction->setEnabled(false);
  newOperationAction->setEnabled(false);  // only enable when we have an internet connection
  chooseOpSubmenu->addSeparator();

  // settings submenu

  addToMenu(tr("Export Data"), &exportAction, &importExportSubmenu);
  addToMenu(tr("Import Data"), &importAction, &importExportSubmenu);

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
  connect(exportAction, actTriggered, this, [this, toTop](){toTop(exportWindow);});
  connect(importAction, actTriggered, this, [this, toTop](){toTop(importWindow);});
  connect(showSettingsAction, actTriggered, this, [this, toTop](){toTop(settingsWindow);});
  connect(captureScreenAreaAction, actTriggered, this, &TrayManager::captureAreaActionTriggered);
  connect(captureWindowAction, actTriggered, this, &TrayManager::captureWindowActionTriggered);
  connect(showEvidenceManagerAction, actTriggered, this, [this, toTop](){toTop(evidenceManagerWindow);});
  connect(showCreditsAction, actTriggered, this, [this, toTop](){toTop(creditsWindow);});
  connect(addCodeblockAction, actTriggered, this, &TrayManager::captureCodeblockActionTriggered);
  connect(newOperationAction, actTriggered, this, [this, toTop](){toTop(createOperationWindow);});

  connect(exportWindow, &PortingDialog::portCompleted, this, [this](const QString& path) {
    openServicesPath = path;
    setTrayMessage(OPEN_PATH, tr("Export Complete"), tr("Export saved to: %1\nClick to view").arg(path));
  });
  connect(importWindow, &PortingDialog::portCompleted, this, [this](const QString& path) {
    setTrayMessage(NO_ACTION, tr("Import Complete"), tr("Import retrieved from: %1").arg(path));
  });

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
  
  connect(trayIcon, &QSystemTrayIcon::messageClicked, this, &TrayManager::onTrayMessageClicked);
  connect(trayIcon, &QSystemTrayIcon::activated, this, [this] {
    chooseOpStatusAction->setText(tr("Loading operations..."));
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
  connect(getInfoWindow, &GetInfo::evidenceSubmitted, [](const model::Evidence& evi){
    AppSettings::getInstance().setLastUsedTags(evi.tags);
  });
  getInfoWindow->show();
}

qint64 TrayManager::createNewEvidence(const QString& filepath, const QString& evidenceType) {
  AppSettings& inst = AppSettings::getInstance();
  auto evidenceID = db->createEvidence(filepath, inst.operationSlug(), evidenceType);
  auto tags = inst.getLastUsedTags();
  if (!tags.empty()) {
    db->setEvidenceTags(tags, evidenceID);
  }
  return evidenceID;
}

void TrayManager::captureWindowActionTriggered() {
  if(AppSettings::getInstance().operationSlug().isEmpty()) {
    showNoOperationSetTrayMessage();
    return;
  }
  screenshotTool->captureWindow();
}

void TrayManager::captureAreaActionTriggered() {
  if(AppSettings::getInstance().operationSlug().isEmpty()) {
    showNoOperationSetTrayMessage();
    return;
  }
  screenshotTool->captureArea();
}

void TrayManager::captureCodeblockActionTriggered() {
  if(AppSettings::getInstance().operationSlug().isEmpty()) {
    showNoOperationSetTrayMessage();
    return;
  }
  onCodeblockCapture();
}

void TrayManager::onCodeblockCapture() {
  QString clipboardContent = ClipboardHelper::readPlaintext();
  if (!clipboardContent.isEmpty()) {
    Codeblock evidence(clipboardContent);
    Codeblock::saveCodeblock(evidence);
    try {
      auto evidenceID = createNewEvidence(evidence.filePath(), QStringLiteral("codeblock"));
      spawnGetInfoWindow(evidenceID);
    }
    catch (QSqlError& e) {
      std::cout << "could not write to the database: " << e.text().toStdString() << std::endl;
    }
  }
}

void TrayManager::onScreenshotCaptured(const QString& path) {
  try {
    auto evidenceID = createNewEvidence(path, QStringLiteral("image"));
    spawnGetInfoWindow(evidenceID);
  }
  catch (QSqlError& e) {
    std::cout << "could not write to the database: " << e.text().toStdString() << std::endl;
  }
}

void TrayManager::showNoOperationSetTrayMessage() {
  setTrayMessage(NO_ACTION, tr("Unable to Record Evidence"),
                        tr("No Operation has been selected. Please select an operation first."),
                        QSystemTrayIcon::Warning);
}

void TrayManager::setActiveOperationLabel() {
  const auto& opName = AppSettings::getInstance().operationName();
  currentOperationMenuAction->setText(tr("Operation: %1").arg(opName.isEmpty() ? tr("<None>") : opName));
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

      connect(newAction, &QAction::triggered, this, [this, newAction, op] {
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
      AppSettings::getInstance().setOperationDetails(QString(), QString());
    }
  }
  else {
    chooseOpStatusAction->setText(tr("Unable to load operations"));
  }
}

void TrayManager::checkForUpdate() {
  NetMan::getInstance().checkForNewRelease(Constants::releaseOwner(), Constants::releaseRepo());
}

void TrayManager::onReleaseCheck(bool success, const std::vector<dto::GithubRelease>& releases) {
  if (!success) {
    return;  // doesn't matter if this fails -- another request will be made later.
  }

  auto digest = dto::ReleaseDigest::fromReleases(Constants::releaseTag(), releases);

  if (digest.hasUpgrade()) {
    setTrayMessage(UPGRADE, tr("A new version is available!"), tr("Click for more info"));
  }
}

void TrayManager::setTrayMessage(MessageType type, const QString& title, const QString& message,
                                 QSystemTrayIcon::MessageIcon icon, int millisecondsTimeoutHint) {
  trayIcon->showMessage(title, message, icon, millisecondsTimeoutHint);
  this->currentTrayMessage = type;
}

void TrayManager::onTrayMessageClicked() {
  switch(currentTrayMessage) {
    case UPGRADE:
      QDesktopServices::openUrl(Constants::releasePageUrl());
      break;
    case OPEN_PATH:
      QDesktopServices::openUrl(openServicesPath);
    case NO_ACTION:
    default:
      break;
  }
}

#endif
