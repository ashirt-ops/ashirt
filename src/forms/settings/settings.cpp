// Copyright 2020, Verizon Media
// Licensed under the terms of MIT. See LICENSE file in project root for terms.

#include "settings.h"

#include <QDateTime>
#include <QFileDialog>
#include <QKeySequence>
#include <QString>

#include "appconfig.h"
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
  delete _captureAreaCmdLabel;
  delete _captureAreaShortcutLabel;
  delete _captureWindowCmdLabel;
  delete _captureWindowShortcutLabel;
  delete _recordCodeblockShortcutLabel;

  delete eviRepoTextBox;
  delete captureAreaCmdTextBox;
  delete captureAreaShortcutTextBox;
  delete captureWindowCmdTextBox;
  delete captureWindowShortcutTextBox;
  delete recordCodeblockShortcutTextBox;
  delete eviRepoBrowseButton;
  delete buttonBox;

  delete gridLayout;

  delete couldNotSaveSettingsMsg;
  delete closeWindowAction;
}

void Settings::buildUi() {
  gridLayout = new QGridLayout(this);
  _eviRepoLabel = new QLabel("Evidence Repository", this);
  _captureAreaCmdLabel = new QLabel("Capture Area Command", this);
  _captureAreaShortcutLabel = new QLabel("Shortcut", this);
  _captureWindowCmdLabel = new QLabel("Capture Window Command", this);
  _captureWindowShortcutLabel = new QLabel("Shortcut", this);
  _recordCodeblockShortcutLabel = new QLabel("Record Codeblock Shortcut", this);

  eviRepoTextBox = new QLineEdit(this);
  captureAreaCmdTextBox = new QLineEdit(this);
  captureAreaShortcutTextBox = new SingleStrokeKeySequenceEdit(this);
  captureWindowCmdTextBox = new QLineEdit(this);
  captureWindowShortcutTextBox = new SingleStrokeKeySequenceEdit(this);
  recordCodeblockShortcutTextBox = new SingleStrokeKeySequenceEdit(this);
  eviRepoBrowseButton = new QPushButton("Browse", this);
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
    1  | Cap A Cmd Lbl | [CapACmdTB] | CapASh lbl | [CapASh TB] |
       +---------------+-------------+------------+-------------+
    2  | Cap W Cmd Lbl | [CapWCmdTB] | CapWSh lbl | [CapWSh TB] |
       +---------------+-------------+------------+-------------+
    3  | CodeblkSh Lbl | [CodeblkSh TB] |                       |
       +---------------+-------------+------------+-------------+
    4  | Vertical spacer                                        |
       +---------------+-------------+------------+-------------+
    5  | Dialog button Box{save, cancel}                        |
       +---------------+-------------+------------+-------------+
  */

  // row 0
  int row = 0;
  gridLayout->addWidget(_eviRepoLabel, row, 0);
  gridLayout->addWidget(eviRepoTextBox, row, 1, 1, 3);
  gridLayout->addWidget(eviRepoBrowseButton, row, 4);

  // row 1
  row++;
  gridLayout->addWidget(_captureAreaCmdLabel, row, 0);
  gridLayout->addWidget(captureAreaCmdTextBox, row, 1);
  gridLayout->addWidget(_captureAreaShortcutLabel, row, 2);
  gridLayout->addWidget(captureAreaShortcutTextBox, row, 3, 1, 2);

  // row 2
  row++;
  gridLayout->addWidget(_captureWindowCmdLabel, row, 0);
  gridLayout->addWidget(captureWindowCmdTextBox, row, 1);
  gridLayout->addWidget(_captureWindowShortcutLabel, row, 2);
  gridLayout->addWidget(captureWindowShortcutTextBox, row, 3, 1, 2);

  // row 3
  row++;
  gridLayout->addWidget(_recordCodeblockShortcutLabel, row, 0);
  gridLayout->addWidget(recordCodeblockShortcutTextBox, row, 1);

  // row 4
  row++;
  gridLayout->addItem(spacer, row, 0, 1, gridLayout->columnCount());

  // row 5
  row++;
  gridLayout->addWidget(buttonBox, row, 0, 1, gridLayout->columnCount());

  closeWindowAction = new QAction(this);
  closeWindowAction->setShortcut(QKeySequence::Close);
  this->addAction(closeWindowAction);

  this->setLayout(gridLayout);
  this->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);
  this->resize(760, 175);
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
  connect(eviRepoBrowseButton, &QPushButton::clicked, this, &Settings::onBrowseClicked);
  connect(closeWindowAction, &QAction::triggered, this, &Settings::onSaveClicked);
}

void Settings::showEvent(QShowEvent *evt) {
  QDialog::showEvent(evt);
  auto& cfg = AppConfig::getInstance();
  eviRepoTextBox->setFocus(); //setting focus to prevent retaining focus for macs

  // reset the form in case a user left junk in the text boxes and pressed "cancel"
  eviRepoTextBox->setText(QDir::toNativeSeparators(cfg.evidenceRepo()));
  captureAreaCmdTextBox->setText(cfg.captureScreenAreaCmd());
  captureAreaShortcutTextBox->setKeySequence(QKeySequence::fromString(cfg.captureScreenAreaShortcut()));
  captureWindowCmdTextBox->setText(cfg.captureScreenWindowCmd());
  captureWindowShortcutTextBox->setKeySequence(QKeySequence::fromString(cfg.captureScreenWindowShortcut()));
  recordCodeblockShortcutTextBox->setKeySequence(QKeySequence::fromString(cfg.captureCodeblockShortcut()));
}

void Settings::closeEvent(QCloseEvent *event) {
  onSaveClicked();
  QDialog::closeEvent(event);
}

void Settings::onCancelClicked() {
  reject();
}

void Settings::onSaveClicked() {
  auto &cfg = AppConfig::getInstance();

  cfg.setEvidenceRepo(QDir::fromNativeSeparators(eviRepoTextBox->text()));
  cfg.setCaptureScreenAreaCmd(captureAreaCmdTextBox->text());
  cfg.setCaptureScreenAreaShortcut(captureAreaShortcutTextBox->keySequence().toString());
  cfg.setCaptureScreenWindowCmd(captureWindowCmdTextBox->text());
  cfg.setCaptureScreenWindowShortcut(captureWindowShortcutTextBox->keySequence().toString());
  cfg.setCaptureCodeblockShortcut(recordCodeblockShortcutTextBox->keySequence().toString());

  try {
    AppConfig::getInstance().writeConfig();
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
