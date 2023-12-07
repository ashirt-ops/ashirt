#include "traymanager.h"

#ifndef QT_NO_SYSTEMTRAYICON

#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QCloseEvent>
#include <QCoreApplication>
#include <QMenu>
#include <QMessageBox>
#include <QMimeData>
#include <QTimer>
#include <QDesktopServices>
#include <iostream>
#include "appconfig.h"
#include "db/databaseconnection.h"
#include "forms/getinfo/getinfo.h"
#include "helpers/netman.h"
#include "helpers/screenshot.h"
#include "helpers/releaseinfo.h"
#include "helpers/system_helpers.h"
#include "hotkeymanager.h"
#include "models/codeblock.h"
#include "firstRunWizard/firstTimeWizard.h"
#include "firstRunWizard/welcomepage.h"

TrayManager::TrayManager(QWidget * parent, DatabaseConnection* db)
    : QDialog(parent)
    , db(db)
    , screenshotTool(new Screenshot(this))
    , updateCheckTimer(new QTimer(this))
    , settingsWindow(new Settings(this))
    , evidenceManagerWindow(new EvidenceManager(this->db, this))
    , creditsWindow(new Credits(this))
    , importWindow(new PortingDialog(PortingDialog::Import, this->db, this))
    , exportWindow(new PortingDialog(PortingDialog::Export, this->db, this))
    , createOperationWindow(new CreateOperation(this))
    , newOperationAction(new QAction(tr("Connect to server first"), this))
    , trayIcon(new QSystemTrayIcon(getTrayIcon(),this))
    , allOperationActions(this)

{
  HotkeyManager::updateHotkeys();
  updateCheckTimer->start(MS_IN_DAY); // every day

  buildUi();
  wireUi();

  // delayed so that windows can listen for get all ops signal
  NetMan::refreshOperationsList();
  QTimer::singleShot(5000, this, &TrayManager::checkForUpdate);

  if(AppConfig::value(CONFIG::SHOW_WELCOME_SCREEN) != "false")
    showWelcomeScreen();
}

TrayManager::~TrayManager() {
  setVisible(false);
  cleanChooseOpSubmenu();
}

void TrayManager::buildUi() {
  //Disable Actions
  newOperationAction->setEnabled(false);  // only enable when we have an internet connection

  // Build Tray menu
  auto trayIconMenu = new QMenu(this);
  trayIconMenu->addAction(tr("Capture from Clipboard"), this, &TrayManager::captureClipboardActionTriggered);
  trayIconMenu->addAction(tr("Capture Screen Area"), this, &TrayManager::captureAreaActionTriggered);
  trayIconMenu->addAction(tr("Capture Window"), this, &TrayManager::captureWindowActionTriggered);
  trayIconMenu->addAction(tr("View Accumulated Evidence"), evidenceManagerWindow, &EvidenceManager::show);
  trayIconMenu->addSeparator();
  chooseOpSubmenu = trayIconMenu->addMenu(tr("Select Operation"));
  trayIconMenu->addSeparator();
  auto importExportSubmenu = trayIconMenu->addMenu(tr("Import/Export"));
  trayIconMenu->addAction(tr("Settings"), settingsWindow, &Settings::show);
  trayIconMenu->addAction(tr("About"), creditsWindow, &Credits::show);
  trayIconMenu->addAction(tr("Quit"), qApp, &QCoreApplication::quit);

  // Operations Submenu
  chooseOpSubmenu->addAction(newOperationAction);
  chooseOpSubmenu->addSeparator();

  // settings submenu
  importExportSubmenu->addAction(tr("Export Data"), exportWindow, &PortingDialog::show);
  importExportSubmenu->addAction(tr("Import Data"), importWindow, &PortingDialog::show);

  setActiveOperationLabel();

  trayIcon->setContextMenu(trayIconMenu);
  trayIcon->show();
}

void TrayManager::wireUi() {
  connect(newOperationAction, &QAction::triggered, createOperationWindow, &CreateOperation::show);

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
  connect(HotkeyManager::get(), &HotkeyManager::clipboardHotkeyPressed, this,
          &TrayManager::captureClipboardActionTriggered);
  connect(HotkeyManager::get(), &HotkeyManager::captureAreaHotkeyPressed, this,
          &TrayManager::captureAreaActionTriggered);
  connect(HotkeyManager::get(), &HotkeyManager::captureWindowHotkeyPressed, this,
          &TrayManager::captureWindowActionTriggered);

  // connect to network signals
  connect(NetMan::get(), &NetMan::operationListUpdated, this, &TrayManager::onOperationListUpdated);
  connect(NetMan::get(), &NetMan::releasesChecked, this, &TrayManager::onReleaseCheck);
  connect(AppConfig::get(), &AppConfig::operationChanged, this, &TrayManager::setActiveOperationLabel);

  connect(trayIcon, &QSystemTrayIcon::messageClicked, this, &TrayManager::onTrayMessageClicked);
  connect(trayIcon, &QSystemTrayIcon::activated, this, [this] {
    newOperationAction->setEnabled(false);
    NetMan::refreshOperationsList();
  });

  connect(updateCheckTimer, &QTimer::timeout, this, &TrayManager::checkForUpdate);
}

void TrayManager::cleanChooseOpSubmenu() {
  // delete all of the existing events
  for (auto act : allOperationActions.actions()) {
    chooseOpSubmenu->removeAction(act);
    allOperationActions.removeAction(act);
  }
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

void TrayManager::changeEvent(QEvent *event)
{
  if (event->type() == QEvent::PaletteChange) {
    trayIcon->setIcon(getTrayIcon());
    event->accept();
  }
  event->ignore();
}

void TrayManager::showWizard()
{
    auto gandalf = new FirstTimeWizard();
    gandalf->show();
}

void TrayManager::showWelcomeScreen()
{
  auto welcomePage = new WelcomePage(this);
  connect(welcomePage, &WelcomePage::requestSetupWizard, this, &TrayManager::showWizard);
  welcomePage->show();
}

void TrayManager::spawnGetInfoWindow(qint64 evidenceID) {
  auto getInfoWindow = new GetInfo(db, evidenceID, this);
  connect(getInfoWindow, &GetInfo::evidenceSubmitted, [](const model::Evidence& evi) {
    AppConfig::setLastUsedTags(evi.tags);
  });
  getInfoWindow->show();
}

qint64 TrayManager::createNewEvidence(const QString& filepath, const QString& evidenceType) {
  auto evidenceID = db->createEvidence(filepath, AppConfig::operationSlug(), evidenceType);
  auto tags = AppConfig::getLastUsedTags();
  db->setEvidenceTags(tags, evidenceID);
  return evidenceID;
}

void TrayManager::captureWindowActionTriggered() {
  if(AppConfig::operationSlug().isEmpty()) {
    showNoOperationSetTrayMessage();
    return;
  }
  screenshotTool->captureWindow();
}

void TrayManager::captureAreaActionTriggered() {
  if(AppConfig::operationSlug().isEmpty()) {
    showNoOperationSetTrayMessage();
    return;
  }
  screenshotTool->captureArea();
}

void TrayManager::captureClipboardActionTriggered() {
  if(AppConfig::operationSlug().isEmpty()) {
    showNoOperationSetTrayMessage();
    return;
  }
  onClipboardCapture();
}
void TrayManager::onClipboardCapture()
{
    const QMimeData *mimeData = QApplication::clipboard()->mimeData();
    QString path;
    QString type;
    if (mimeData->hasHtml() || mimeData->hasText()) {
        QString clipboardContent = mimeData->text();
        if (clipboardContent.isEmpty())
            return;

        Codeblock evidence(clipboardContent);
        if(!Codeblock::saveCodeblock(evidence)) {
            setTrayMessage(NO_ACTION, _recordErrorTitle, tr("Error Gathering Evidence from clipboard"), QSystemTrayIcon::Information);
            return;
        }
        path = evidence.filePath();
        type = Codeblock::contentType();
    } else if (mimeData->hasImage()) {
        path  = QDir::toNativeSeparators(SystemHelpers::pathToEvidence().append(Screenshot::mkName()));
        QImage img = qvariant_cast<QImage>(mimeData->imageData());
        img.save(path);
        type = Screenshot::contentType();
    } else {
        return;
    }

    int evidenceID = createNewEvidence(path, type);
    if(evidenceID == -1) {
        showDBWriteErrorTrayMessage();
        return;
    }
    spawnGetInfoWindow(evidenceID);
}

void TrayManager::onScreenshotCaptured(const QString& path)
{
  auto evidenceID = createNewEvidence(path, QStringLiteral("image"));
  if(evidenceID == -1) {
      showDBWriteErrorTrayMessage();
      return;
  }
    spawnGetInfoWindow(evidenceID);
}

void TrayManager::showDBWriteErrorTrayMessage()
{
    setTrayMessage(NO_ACTION, _recordErrorTitle,tr("Could not write to database"), QSystemTrayIcon::Warning);
}

void TrayManager::showNoOperationSetTrayMessage() {
  setTrayMessage(NO_ACTION, _recordErrorTitle,
                        tr("No Operation has been selected. Please select an operation first."),
                        QSystemTrayIcon::Warning);
}

void TrayManager::setActiveOperationLabel() {
  const auto& opName = AppConfig::operationName();
  chooseOpSubmenu->setTitle(tr("Operation: %1").arg(opName.isEmpty() ? tr("<None>") : opName));
  for(const auto &a : allOperationActions.actions()) {
      if(a->text() == opName) {
          a->setChecked(true);
          selectedAction = a;
      }
  }
}

void TrayManager::onOperationListUpdated(bool success,
                                         const QList<dto::Operation>& operations) {

  if (!success)
      return;

  auto currentOp = AppConfig::operationSlug();
  newOperationAction->setEnabled(true);
  newOperationAction->setText(tr("New Operation"));
  cleanChooseOpSubmenu();
  for (const auto& op : operations) {
    auto newAction = std::make_shared<QAction>(new QAction(this));
    newAction->setText(op.name);
    newAction->setCheckable(true);
    if (currentOp == op.slug) {
      newAction->setChecked(true);
      selectedAction = newAction.get();
    }
    allOperationActions.addAction(newAction.get());
    connect(newAction.get(), &QAction::triggered, this, [this, newAction, op] {
      AppConfig::setLastUsedTags(QList<model::Tag>{}); // clear last used tags
      AppConfig::setOperationDetails(op.slug, op.name);
      selectedAction = newAction.get();
    });
    chooseOpSubmenu->addAction(newAction.get());
  }

  if (!selectedAction) {
    AppConfig::setOperationDetails(QString(), QString());
  }
}

void TrayManager::checkForUpdate() {
  NetMan::checkForNewRelease(ReleaseInfo::owner, ReleaseInfo::repo);
}

void TrayManager::onReleaseCheck(bool success, const QList<dto::GithubRelease>& releases) {
  if (!success) {
    return;  // doesn't matter if this fails -- another request will be made later.
  }

  auto digest = dto::ReleaseDigest::fromReleases(ReleaseInfo::version, releases);

  if (digest.hasUpgrade()) {
    setTrayMessage(UPGRADE, tr("A new version is available!"), tr("Click for more info"));
  }
}

void TrayManager::setTrayMessage(MessageType type, const QString& title, const QString& message,
                                 QSystemTrayIcon::MessageIcon icon, int millisecondsTimeoutHint) {
  trayIcon->showMessage(title, message, icon, millisecondsTimeoutHint);
  currentTrayMessage = type;
}

QIcon TrayManager::getTrayIcon()
{
#if defined(Q_OS_MACOS)
  QIcon icon = QIcon(QStringLiteral(":/icons/shirt-dark.svg"));
  icon.setIsMask(true);
#else
  QIcon icon = SystemHelpers::isLightTheme() ?  QIcon(QStringLiteral(":/icons/shirt-dark.svg")) : QIcon(QStringLiteral(":/icons/shirt-light.svg"));
#endif
  return icon;
}

void TrayManager::onTrayMessageClicked() {
  switch(currentTrayMessage) {
    case UPGRADE:
      QDesktopServices::openUrl(ReleaseInfo::releasePageUrl);
      break;
    case OPEN_PATH:
      QDesktopServices::openUrl(openServicesPath);
    case NO_ACTION:
    default:
      break;
  }
}

#endif
