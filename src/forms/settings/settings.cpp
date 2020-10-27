// Copyright 2020, Verizon Media
// Licensed under the terms of MIT. See LICENSE file in project root for terms.

#include "settings.h"

#include <QDateTime>
#include <QFileDialog>
#include <QKeySequence>
#include <QString>

#include "appconfig.h"
#include "appsettings.h"
#include "dtos/checkConnection.h"
#include "helpers/http_status.h"
#include "helpers/netman.h"
#include "helpers/stopreply.h"
#include "helpers/ui_helpers.h"
#include "hotkeymanager.h"
#include "components/custom_keyseq_edit/singlestrokekeysequenceedit.h"

Settings::Settings(DatabaseConnection* db, HotkeyManager *hotkeyManager, QWidget *parent) : QDialog(parent) {
  this->db = db;
  this->hotkeyManager = hotkeyManager;
  buildUi();
  wireUi();
}

Settings::~Settings() {
  delete _eviRepoLabel;
  delete _accessKeyLabel;
  delete _secretKeyLabel;
  delete _hostPathLabel;
  delete _captureAreaCmdLabel;
  delete _captureAreaShortcutLabel;
  delete _captureWindowCmdLabel;
  delete _captureWindowShortcutLabel;
  delete _recordCodeblockShortcutLabel;
  delete connStatusLabel;

  delete eviRepoTextBox;
  delete accessKeyTextBox;
  delete secretKeyTextBox;
  delete hostPathTextBox;
  delete captureAreaCmdTextBox;
  delete captureAreaShortcutTextBox;
  delete captureWindowCmdTextBox;
  delete captureWindowShortcutTextBox;
  delete recordCodeblockShortcutTextBox;
  delete testConnectionButton;
  delete eviRepoBrowseButton;
  delete buttonBox;

  delete gridLayout;

  delete couldNotSaveSettingsMsg;
  stopReply(&currentTestReply);
  delete closeWindowAction;
}

void Settings::buildUi() {
  gridLayout = new QGridLayout(this);
  _eviRepoLabel = new QLabel("Evidence Repository", this);
  _accessKeyLabel = new QLabel("Access Key", this);
  _secretKeyLabel = new QLabel("Secret Key", this);
  _hostPathLabel = new QLabel("Host Path", this);
  _captureAreaCmdLabel = new QLabel("Capture Area Command", this);
  _captureAreaShortcutLabel = new QLabel("Shortcut", this);
  _captureWindowCmdLabel = new QLabel("Capture Window Command", this);
  _captureWindowShortcutLabel = new QLabel("Shortcut", this);
  _recordCodeblockShortcutLabel = new QLabel("Record Codeblock Shortcut", this);
  connStatusLabel = new QLabel("", this);

  eviRepoTextBox = new QLineEdit(this);
  accessKeyTextBox = new QLineEdit(this);
  secretKeyTextBox = new QLineEdit(this);
  hostPathTextBox = new QLineEdit(this);
  captureAreaCmdTextBox = new QLineEdit(this);
  captureAreaShortcutTextBox = new SingleStrokeKeySequenceEdit(this);
  captureWindowCmdTextBox = new QLineEdit(this);
  captureWindowShortcutTextBox = new SingleStrokeKeySequenceEdit(this);
  recordCodeblockShortcutTextBox = new SingleStrokeKeySequenceEdit(this);
  eviRepoBrowseButton = new QPushButton("Browse", this);
  testConnectionButton = new LoadingButton("Test Connection", this);
  buttonBox = new QDialogButtonBox(this);
  buttonBox->addButton(QDialogButtonBox::Save);
  buttonBox->addButton(QDialogButtonBox::Cancel);

  spacer = new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Expanding);
  couldNotSaveSettingsMsg = new QErrorMessage(this);

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
    6  | CodeblkSh Lbl | [CodeblkSh TB] |                       |
       +---------------+-------------+------------+-------------+
    7  | Test Conn Btn |  StatusLabel                           |
       +---------------+-------------+------------+-------------+
    8  | Vertical spacer                                        |
       +---------------+-------------+------------+-------------+
    9  | Dialog button Box{save, cancel}                        |
       +---------------+-------------+------------+-------------+
  */

  // row 0
  gridLayout->addWidget(_eviRepoLabel, 0, 0);
  gridLayout->addWidget(eviRepoTextBox, 0, 1, 1, 3);
  gridLayout->addWidget(eviRepoBrowseButton, 0, 4);

  // row 1
  gridLayout->addWidget(_accessKeyLabel, 1, 0);
  gridLayout->addWidget(accessKeyTextBox, 1, 1, 1, 4);

  // row 2
  gridLayout->addWidget(_secretKeyLabel, 2, 0);
  gridLayout->addWidget(secretKeyTextBox, 2, 1, 1, 4);

  // row 3
  gridLayout->addWidget(_hostPathLabel, 3, 0);
  gridLayout->addWidget(hostPathTextBox, 3, 1, 1, 4);

  // row 4
  gridLayout->addWidget(_captureAreaCmdLabel, 4, 0);
  gridLayout->addWidget(captureAreaCmdTextBox, 4, 1);
  gridLayout->addWidget(_captureAreaShortcutLabel, 4, 2);
  gridLayout->addWidget(captureAreaShortcutTextBox, 4, 3, 1, 2);

  // row 5
  gridLayout->addWidget(_captureWindowCmdLabel, 5, 0);
  gridLayout->addWidget(captureWindowCmdTextBox, 5, 1);
  gridLayout->addWidget(_captureWindowShortcutLabel, 5, 2);
  gridLayout->addWidget(captureWindowShortcutTextBox, 5, 3, 1, 2);

  // row 6 (reserved for codeblocks)
  gridLayout->addWidget(_recordCodeblockShortcutLabel, 6, 0);
  gridLayout->addWidget(recordCodeblockShortcutTextBox, 6, 1);

  // row 7
  gridLayout->addWidget(testConnectionButton, 7, 0);
  gridLayout->addWidget(connStatusLabel, 7, 1, 1, 4);

  // row 8
  gridLayout->addItem(spacer, 8, 0, 1, gridLayout->columnCount());

  // row 9
  gridLayout->addWidget(buttonBox, 9, 0, 1, gridLayout->columnCount());

  closeWindowAction = new QAction(this);
  closeWindowAction->setShortcut(QKeySequence::Close);
  this->addAction(closeWindowAction);

  this->setLayout(gridLayout);
  this->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);
  this->resize(760, 300);
  this->setWindowTitle("Settings");

  // Make the dialog pop up above any other windows but retain title bar and buttons
  Qt::WindowFlags flags = this->windowFlags();
  flags |= Qt::CustomizeWindowHint | Qt::WindowStaysOnTopHint | Qt::WindowMinMaxButtonsHint |
           Qt::WindowCloseButtonHint;
  this->setWindowFlags(flags);
}

void Settings::wireUi() {
  connect(buttonBox, &QDialogButtonBox::accepted, this, &Settings::onSaveClicked);
  connect(buttonBox, &QDialogButtonBox::rejected, this, &Settings::onCancelClicked);
  connect(testConnectionButton, &QPushButton::clicked, this, &Settings::onTestConnectionClicked);
  connect(eviRepoBrowseButton, &QPushButton::clicked, this, &Settings::onBrowseClicked);
  connect(closeWindowAction, &QAction::triggered, this, &Settings::onSaveClicked);
}

void Settings::showEvent(QShowEvent *evt) {
  QDialog::showEvent(evt);
  auto& cfg = AppConfig::getInstance();
  eviRepoTextBox->setFocus(); //setting focus to prevent retaining focus for macs

  // reset the form in case a user left junk in the text boxes and pressed "cancel"
  eviRepoTextBox->setText(QDir::toNativeSeparators(cfg.evidenceRepo()));
  accessKeyTextBox->setText(db->accessKey());
  secretKeyTextBox->setText(db->secretKey());
  hostPathTextBox->setText(db->hostPath());
  captureAreaCmdTextBox->setText(cfg.captureScreenAreaCmd());
  captureAreaShortcutTextBox->setKeySequence(QKeySequence::fromString(cfg.captureScreenAreaShortcut()));
  captureWindowCmdTextBox->setText(cfg.captureScreenWindowCmd());
  captureWindowShortcutTextBox->setKeySequence(QKeySequence::fromString(cfg.captureScreenWindowShortcut()));
  recordCodeblockShortcutTextBox->setKeySequence(QKeySequence::fromString(cfg.captureCodeblockShortcut()));

  // re-enable form
  connStatusLabel->setText("");
  testConnectionButton->setEnabled(true);
}

void Settings::closeEvent(QCloseEvent *event) {
  onSaveClicked();
  QDialog::closeEvent(event);
}

void Settings::onCancelClicked() {
  stopReply(&currentTestReply);
  reject();
}

void Settings::onSaveClicked() {
  stopReply(&currentTestReply);
  connStatusLabel->setText("");

  auto &cfg = AppConfig::getInstance();

  cfg.setEvidenceRepo(QDir::fromNativeSeparators(eviRepoTextBox->text()));
  cfg.setCaptureScreenAreaCmd(captureAreaCmdTextBox->text());
  cfg.setCaptureScreenAreaShortcut(captureAreaShortcutTextBox->keySequence().toString());
  cfg.setCaptureScreenWindowCmd(captureWindowCmdTextBox->text());
  cfg.setCaptureScreenWindowShortcut(captureWindowShortcutTextBox->keySequence().toString());
  cfg.setCaptureCodeblockShortcut(recordCodeblockShortcutTextBox->keySequence().toString());

  try {
    AppConfig::getInstance().writeConfig();
    db->updateServerDetails(accessKeyTextBox->text(), secretKeyTextBox->text(), hostPathTextBox->text());
  }
  catch (std::exception &e) {
    couldNotSaveSettingsMsg->showMessage("Unable to save settings. Error: " + QString(e.what()));
  }

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

void Settings::onTestConnectionClicked() {
  if (hostPathTextBox->text().isEmpty()
      || accessKeyTextBox->text().isEmpty()
      || secretKeyTextBox->text().isEmpty()) {
    connStatusLabel->setText("Please set Access Key, Secret key and Host Path first.");
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
          connStatusLabel->setText("Connected");
        }
        else {
          connStatusLabel->setText("Unable to connect: Wrong or outdated server");
        }
        break;
      case HttpStatus::StatusUnauthorized:
        connStatusLabel->setText("Could not connect: Unauthorized (check access key and secret)");
        break;
      case HttpStatus::StatusNotFound:
        connStatusLabel->setText("Could not connect: Not Found (check URL)");
        break;
      default:
        QString msg = "Could not connect: Unexpected Error (code: %1)";
        connStatusLabel->setText(msg.arg(statusCode));
    }
  }
  else {
    connStatusLabel->setText(
        "Could not connect: Unexpected Error (check network connection and URL)");
  }

  testConnectionButton->stopAnimation();
  testConnectionButton->setEnabled(true);
  tidyReply(&currentTestReply);
}
