// Copyright 2020, Verizon Media
// Licensed under the terms of GPLv3. See LICENSE file in project root for terms.

#include "settings.h"

#include <QDateTime>
#include <QFileDialog>
#include <QString>

#include "appconfig.h"
#include "appsettings.h"
#include "helpers/http_status.h"
#include "helpers/netman.h"
#include "helpers/pathseparator.h"
#include "helpers/stopreply.h"
#include "helpers/ui_helpers.h"
#include "hotkeymanager.h"
#include "ui_settings.h"

using namespace std;

Settings::Settings(HotkeyManager *hotkeyManager, QWidget *parent)
    : ButtonBoxForm(parent), ui(new Ui::Settings) {
  ui->setupUi(this);
  this->hotkeyManager = hotkeyManager;

  couldNotSaveSettingsMsg = new QErrorMessage(this);
  setButtonBox(ui->buttonBox);

  testConnectionButton = new LoadingButton(ui->testHostButton->text(), this, ui->testHostButton);
  UiHelpers::replacePlaceholder(ui->testHostButton, testConnectionButton, ui->gridLayout);

  wireUi();

  // Make the dialog pop up above any other windows but retain title bar and buttons
  Qt::WindowFlags flags = this->windowFlags();
  flags |= Qt::CustomizeWindowHint | Qt::WindowStaysOnTopHint | Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint;
  this->setWindowFlags(flags);
}

Settings::~Settings() {
  delete ui;
  delete couldNotSaveSettingsMsg;
  stopReply(&currentTestReply);
  delete testConnectionButton;
}

void Settings::wireUi() {
  connect(ui->buttonBox, &QDialogButtonBox::clicked, this, &Settings::routeButtonPress);
  connect(testConnectionButton, &QPushButton::clicked, this, &Settings::onTestConnectionClicked);
  connect(ui->eviRepoBrowseButton, &QPushButton::clicked, this, &Settings::onBrowseClicked);
}

void Settings::showEvent(QShowEvent *evt) {
  QDialog::showEvent(evt);
  AppConfig &inst = AppConfig::getInstance();

  // reset the form in case a user left junk in the text boxes and pressed "cancel"
  ui->eviRepoTextBox->setText(inst.evidenceRepo);
  ui->accessKeyTextBox->setText(inst.accessKey);
  ui->secretKeyTextBox->setText(inst.secretKey);
  ui->hostPathTextBox->setText(inst.apiURL);
  ui->screenshotCmdTextBox->setText(inst.screenshotExec);
  ui->screenshotShortcutTextBox->setText(inst.screenshotShortcutCombo);

  ui->captureWindowCmdTextBox->setText(inst.captureWindowExec);
  ui->captureWindowShortCutTextBox->setText(inst.captureWindowShortcut);

  // re-enable form
  testConnectionButton->setEnabled(true);
}

void Settings::closeEvent(QCloseEvent *event) {
  onSaveClicked();
  QDialog::closeEvent(event);
}

void Settings::keyPressEvent(QKeyEvent *evt) {
#ifdef Q_OS_MACOS
  if( evt->key() == Qt::Key_W && evt->modifiers() == Qt::MetaModifier) {
    onCancelClicked();
    close(); // not needed once onCancelClick implements close directly (other branch)
  }
#else
  if( evt->key() == Qt::Key_W && evt->modifiers() == Qt::ControlModifier) {
    onCancelClicked();
    close(); // not needed once onCancelClick implements close directly (other branch)
  }
#endif

  QDialog::keyPressEvent(evt);
}

void Settings::onCancelClicked() {
  stopReply(&currentTestReply);
  ui->statusIconLabel->setText("");
}

void Settings::onSaveClicked() {
  stopReply(&currentTestReply);
  ui->statusIconLabel->setText("");

  AppConfig &inst = AppConfig::getInstance();

  inst.evidenceRepo = ui->eviRepoTextBox->text();
  inst.accessKey = ui->accessKeyTextBox->text();
  inst.secretKey = ui->secretKeyTextBox->text();
  inst.apiURL = ui->hostPathTextBox->text();
  inst.screenshotExec = ui->screenshotCmdTextBox->text();
  inst.screenshotShortcutCombo = ui->screenshotShortcutTextBox->text();
  inst.captureWindowExec = ui->captureWindowCmdTextBox->text();
  inst.captureWindowShortcut = ui->captureWindowShortCutTextBox->text();

  try {
    inst.writeConfig();
  }
  catch (std::exception &e) {
    couldNotSaveSettingsMsg->showMessage("Unable to save settings. Error: " + QString(e.what()));
  }

  hotkeyManager->updateHotkeys();
}

void Settings::onBrowseClicked() {
  auto browseStart = ui->eviRepoTextBox->text();
  browseStart = QFile(browseStart).exists() ? browseStart : QDir::homePath();
  auto filename = QFileDialog::getExistingDirectory(this, tr("Select a project directory"),
                                                    browseStart, QFileDialog::ShowDirsOnly);
  if (filename != nullptr) {
    ui->eviRepoTextBox->setText(filename);
  }
}

void Settings::onTestConnectionClicked() {
  testConnectionButton->startAnimation();
  testConnectionButton->setEnabled(false);
  currentTestReply = NetMan::getInstance().testConnection(
      ui->hostPathTextBox->text(), ui->accessKeyTextBox->text(), ui->secretKeyTextBox->text());
  connect(currentTestReply, &QNetworkReply::finished, this, &Settings::onTestRequestComplete);
}

void Settings::onTestRequestComplete() {
  bool ok = true;
  auto statusCode =
      currentTestReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(&ok);

  if (ok) {
    switch (statusCode) {
      case HttpStatus::StatusOK:
        this->ui->statusIconLabel->setText("Connected");
        break;
      case HttpStatus::StatusUnauthorized:
        this->ui->statusIconLabel->setText(
            "Could not connect: Unauthorized (check api key and secret)");
        break;
      case HttpStatus::StatusNotFound:
        this->ui->statusIconLabel->setText("Could not connect: Not Found (check URL)");
        break;
      default:
        QString msg = "Could not connect: Unexpected Error (code: ";
        msg.append(statusCode);
        msg.append(")");
        this->ui->statusIconLabel->setText(msg);
    }
  }
  else {
    this->ui->statusIconLabel->setText(
        "Could not connect: Unexpected Error (check network connection and URL)");
  }

  testConnectionButton->stopAnimation();
  testConnectionButton->setEnabled(true);
  tidyReply(&currentTestReply);
}
