// Copyright 2020, Verizon Media
// Licensed under the terms of MIT. See LICENSE file in project root for terms.

#include "settings.h"

#include <QDateTime>
#include <QDialogButtonBox>
#include <QErrorMessage>
#include <QFileDialog>
#include <QGridLayout>
#include <QKeySequence>
#include <QKeySequenceEdit>
#include <QLabel>
#include <QLineEdit>
#include <QNetworkReply>
#include <QPushButton>
#include <QString>

#include "appconfig.h"
#include "appsettings.h"
#include "dtos/checkConnection.h"
#include "helpers/http_status.h"
#include "helpers/netman.h"
#include "helpers/stopreply.h"
#include "hotkeymanager.h"
#include "components/custom_keyseq_edit/singlestrokekeysequenceedit.h"
#include "components/loading_button/loadingbutton.h"

Settings::Settings(HotkeyManager *hotkeyManager, QWidget *parent)
    : AShirtDialog(parent, AShirtDialog::commonWindowFlags)
    , hotkeyManager(hotkeyManager)
    , connStatusLabel(new QLabel(this))
    , eviRepoTextBox(new QLineEdit(this))
    , accessKeyTextBox(new QLineEdit(this))
    , secretKeyTextBox(new QLineEdit(this))
    , hostPathTextBox(new QLineEdit(this))
    , captureAreaCmdTextBox(new QLineEdit(this))
    , captureAreaShortcutTextBox(new SingleStrokeKeySequenceEdit(this))
    , captureWindowCmdTextBox(new QLineEdit(this))
    , captureWindowShortcutTextBox(new SingleStrokeKeySequenceEdit(this))
    , captureClipboardShortcutTextBox(new SingleStrokeKeySequenceEdit(this))
    , testConnectionButton(new LoadingButton(tr("Test Connection"), this))
    , couldNotSaveSettingsMsg(new QErrorMessage(this))
{
  buildUi();
  wireUi();
}

Settings::~Settings() {
  stopReply(&currentTestReply);
}

void Settings::buildUi() {
  auto eviRepoBrowseButton = new QPushButton(tr("Browse"), this);
  connect(eviRepoBrowseButton, &QPushButton::clicked, this, &Settings::onBrowseClicked);

  auto clearHotkeysButton = new QPushButton(tr("Clear Shortcuts"), this);
  connect(clearHotkeysButton, &QPushButton::clicked, this, &Settings::onClearShortcutsClicked);

  auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel, this);
  connect(buttonBox, &QDialogButtonBox::accepted, this, &Settings::onSaveClicked);
  connect(buttonBox, &QDialogButtonBox::rejected, this, &Settings::onCancelClicked);
  // Layout
  /*        0                 1           2             3
       +---------------+-------------+------------+-------------+
    0  | Evi Repo Lbl  |    [Evi Repo TB]         | browseBtn   |
       +---------------+-------------+------------+-------------+
    1  | A. Key Label  |  [A. Key TB]                           |
       +---------------+-------------+------------+-------------+
    2  | S. Key Label  |  [S. Key TB]                           |
       +---------------+-------------+------------+-------------+
    3  | Host Label    |  [Host TB]                             |
       +---------------+-------------+------------+-------------+
    4  | Cap A Cmd Lbl | [CapACmdTB] | CapASh lbl | [CapASh TB] |
       +---------------+-------------+------------+-------------+
    5  | Cap W Cmd Lbl | [CapWCmdTB] | CapWSh lbl | [CapWSh TB] |
       +---------------+-------------+------------+-------------+
    6  | CodeblkSh Lbl | [CodeblkSh TB] |  Clear Hotkey Btn     |
       +---------------+-------------+------------+-------------+
    7  | Test Conn Btn |  StatusLabel                           |
       +---------------+-------------+------------+-------------+
    8  | Vertical spacer                                        |
       +---------------+-------------+------------+-------------+
    9  | Dialog button Box{save, cancel}                        |
       +---------------+-------------+------------+-------------+
  */
  auto gridLayout = new QGridLayout(this);
  // row 0
  gridLayout->addWidget(new QLabel(tr("Evidence Repository"), this), 0, 0);
  gridLayout->addWidget(eviRepoTextBox, 0, 1, 1, 3);
  gridLayout->addWidget(eviRepoBrowseButton, 0, 4);

  // row 1
  gridLayout->addWidget(new QLabel(tr("Access Key"), this), 1, 0);
  gridLayout->addWidget(accessKeyTextBox, 1, 1, 1, 4);

  // row 2
  gridLayout->addWidget(new QLabel(tr("Secret Key"), this), 2, 0);
  gridLayout->addWidget(secretKeyTextBox, 2, 1, 1, 4);

  // row 3
  gridLayout->addWidget(new QLabel(tr("Host Path"), this), 3, 0);
  gridLayout->addWidget(hostPathTextBox, 3, 1, 1, 4);

  // row 4
  gridLayout->addWidget(new QLabel(tr("Capture Area Command"), this), 4, 0);
  gridLayout->addWidget(captureAreaCmdTextBox, 4, 1);
  gridLayout->addWidget(new QLabel(tr("Shortcut"), this), 4, 2);
  gridLayout->addWidget(captureAreaShortcutTextBox, 4, 3, 1, 2);

  // row 5
  gridLayout->addWidget(new QLabel(tr("Capture Window Command"), this), 5, 0);
  gridLayout->addWidget(captureWindowCmdTextBox, 5, 1);
  gridLayout->addWidget(new QLabel(tr("Shortcut"), this), 5, 2);
  gridLayout->addWidget(captureWindowShortcutTextBox, 5, 3, 1, 2);

  // row 6 (reserved for codeblocks)
  gridLayout->addWidget(new QLabel(tr("Capture Clipboard Shortcut"), this), 6, 0);
  gridLayout->addWidget(captureClipboardShortcutTextBox, 6, 1);
  gridLayout->addWidget(clearHotkeysButton, 6, 2, 1, 3, Qt::AlignRight);

  // row 7
  gridLayout->addWidget(testConnectionButton, 7, 0);
  gridLayout->addWidget(connStatusLabel, 7, 1, 1, 4);

  // row 8
  gridLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Expanding), 8, 0, 1, gridLayout->columnCount());

  // row 9
  gridLayout->addWidget(buttonBox, 9, 0, 1, gridLayout->columnCount());

  setLayout(gridLayout);
  setSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);
  resize(760, 300);
  setWindowTitle(tr("Settings"));
}

void Settings::wireUi() {
  connect(testConnectionButton, &QPushButton::clicked, this, &Settings::onTestConnectionClicked);
  connect(captureAreaShortcutTextBox, &QKeySequenceEdit::keySequenceChanged, this, [this](const QKeySequence &keySequence){
    checkForDuplicateShortcuts(keySequence, captureAreaShortcutTextBox);
  });
  connect(captureWindowShortcutTextBox, &QKeySequenceEdit::keySequenceChanged, this, [this](const QKeySequence &keySequence){
    checkForDuplicateShortcuts(keySequence, captureWindowShortcutTextBox);
  });
  connect(captureClipboardShortcutTextBox, &QKeySequenceEdit::keySequenceChanged, this, [this](const QKeySequence &keySequence){
    checkForDuplicateShortcuts(keySequence, captureClipboardShortcutTextBox);
  });
}

void Settings::checkForDuplicateShortcuts(const QKeySequence& keySequence, QKeySequenceEdit* parentComponent) {
  // these events are generated for every key sequence change, but all except the last are blank
  if(keySequence.isEmpty()) {
    return;
  }

  auto usesKeySequence = [keySequence, parentComponent](QKeySequenceEdit* keySequenceEdit) {
    return parentComponent != keySequenceEdit && // check that we don't compare to itself
           keySequenceEdit->keySequence() == keySequence;
  };

  bool alreadyUsed = usesKeySequence(captureWindowShortcutTextBox)
                     || usesKeySequence(captureClipboardShortcutTextBox)
                     || usesKeySequence(captureAreaShortcutTextBox);

  if(alreadyUsed) {
    parentComponent->clear();
    parentComponent->setStyleSheet(QStringLiteral("background-color: lightcoral"));
  }
  else {
    parentComponent->setStyleSheet(QString());
  }
}

void Settings::showEvent(QShowEvent *evt) {
  QDialog::showEvent(evt);
  hotkeyManager->disableHotkeys();
  
  AppConfig &inst = AppConfig::getInstance();
  eviRepoTextBox->setFocus(); //setting focus to prevent retaining focus for macs

  // reset the form in case a user left junk in the text boxes and pressed "cancel"
  eviRepoTextBox->setText(QDir::toNativeSeparators(inst.evidenceRepo));
  accessKeyTextBox->setText(inst.accessKey);
  secretKeyTextBox->setText(inst.secretKey);
  hostPathTextBox->setText(inst.apiURL);
  captureAreaCmdTextBox->setText(inst.screenshotExec);
  captureAreaShortcutTextBox->setKeySequence(QKeySequence::fromString(inst.screenshotShortcutCombo));
  captureWindowCmdTextBox->setText(inst.captureWindowExec);
  captureWindowShortcutTextBox->setKeySequence(QKeySequence::fromString(inst.captureWindowShortcut));
  captureClipboardShortcutTextBox->setKeySequence(QKeySequence::fromString(inst.captureClipboardShortcut));

  // re-enable form
  connStatusLabel->clear();
  testConnectionButton->setEnabled(true);
}

void Settings::closeEvent(QCloseEvent *event) {
  onSaveClicked();
  hotkeyManager->enableHotkeys();
  QDialog::closeEvent(event);
}

void Settings::onCancelClicked() {
  stopReply(&currentTestReply);
  hotkeyManager->enableHotkeys();
  reject();
}

void Settings::onSaveClicked() {
  stopReply(&currentTestReply);
  connStatusLabel->clear();

  AppConfig &inst = AppConfig::getInstance();

  inst.evidenceRepo = QDir::fromNativeSeparators(eviRepoTextBox->text());
  inst.accessKey = accessKeyTextBox->text();
  inst.secretKey = secretKeyTextBox->text();

  QString originalApiUrl = inst.apiURL;
  inst.apiURL = hostPathTextBox->text();
  if (originalApiUrl != hostPathTextBox->text()) {
    NetMan::getInstance().refreshOperationsList();
  }

  inst.screenshotExec = captureAreaCmdTextBox->text();
  inst.screenshotShortcutCombo = captureAreaShortcutTextBox->keySequence().toString();
  inst.captureWindowExec = captureWindowCmdTextBox->text();
  inst.captureWindowShortcut = captureWindowShortcutTextBox->keySequence().toString();
  inst.captureClipboardShortcut = captureClipboardShortcutTextBox->keySequence().toString();

  if(!inst.writeConfig())
    couldNotSaveSettingsMsg->showMessage(tr("Unable to save settings. Error: %1").arg(inst.errorText));

  hotkeyManager->updateHotkeys();
  close();
}

void Settings::onBrowseClicked() {
  auto browseStart = eviRepoTextBox->text();
  browseStart = QFile(browseStart).exists() ? browseStart : QDir::homePath();
  auto filename = QFileDialog::getExistingDirectory(this, tr("Select a project directory"),
                                                    browseStart, QFileDialog::ShowDirsOnly);
  if (filename != nullptr) {
    eviRepoTextBox->setText(QDir::toNativeSeparators(filename));
  }
}

void Settings::onClearShortcutsClicked() {
  captureAreaShortcutTextBox->clear();
  captureWindowShortcutTextBox->clear();
  captureClipboardShortcutTextBox->clear();
}

void Settings::onTestConnectionClicked() {
  if (hostPathTextBox->text().isEmpty()
      || accessKeyTextBox->text().isEmpty()
      || secretKeyTextBox->text().isEmpty()) {
    connStatusLabel->setText(tr("Please set Access Key, Secret key and Host Path first."));
    return;
  }
  testConnectionButton->startAnimation();
  testConnectionButton->setEnabled(false);
  currentTestReply = NetMan::getInstance().testConnection(
      hostPathTextBox->text(), accessKeyTextBox->text(), secretKeyTextBox->text());
  connect(currentTestReply, &QNetworkReply::finished, this, &Settings::onTestRequestComplete);
}

void Settings::onTestRequestComplete() {
  bool ok = true;
  auto statusCode =
      currentTestReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(&ok);

  if (ok) {
    dto::CheckConnection connectionCheckResp;

    switch (statusCode) {
      case HttpStatus::StatusOK:
        connectionCheckResp = dto::CheckConnection::parseJson(currentTestReply->readAll());
        if (connectionCheckResp.parsedCorrectly && connectionCheckResp.ok) {
          connStatusLabel->setText(tr("Connected"));
        }
        else {
          connStatusLabel->setText(tr("Unable to connect: Wrong or outdated server"));
        }
        break;
      case HttpStatus::StatusUnauthorized:
        connStatusLabel->setText(tr("Could not connect: Unauthorized (check access key and secret)"));
        break;
      case HttpStatus::StatusNotFound:
        connStatusLabel->setText(tr("Could not connect: Not Found (check URL)"));
        break;
      default:
        connStatusLabel->setText(tr("Could not connect: Unexpected Error (code: %1)").arg(statusCode));
    }
  }
  else {
    connStatusLabel->setText(tr("Could not connect: Unexpected Error (check network connection and URL)"));
  }

  testConnectionButton->stopAnimation();
  testConnectionButton->setEnabled(true);
  tidyReply(&currentTestReply);
}
