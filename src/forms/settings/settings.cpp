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
#include "helpers/netman.h"
#include "hotkeymanager.h"
#include "components/custom_keyseq_edit/singlestrokekeysequenceedit.h"
#include "components/loading_button/loadingbutton.h"

Settings::Settings(QWidget *parent)
    : AShirtDialog(parent, AShirtDialog::commonWindowFlags)
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
    , showWelcomeScreen(new QCheckBox(tr("Show Welcome Screen"), this))
{
  buildUi();
  wireUi();
}

void Settings::buildUi() {
  auto eviRepoBrowseButton = new QPushButton(tr("Browse"), this);
  connect(eviRepoBrowseButton, &QPushButton::clicked, this, &Settings::onBrowseClicked);

  auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel, this);
  connect(buttonBox, &QDialogButtonBox::accepted, this, &Settings::onSaveClicked);
  connect(buttonBox, &QDialogButtonBox::rejected, this, &Settings::onCancelClicked);
  // Layout
  /*        0                 1           2             3
       +---------------+-------------+------------+-------------+
    0  | Evi Repo Lbl  |    [Evi Repo TB]         | browseBtn   |
       +---------------+-------------+------------+-------------+
    1  | Host Label    |  [Host TB]                             |
       +---------------+-------------+------------+-------------+
    2  | A. Key Label  |  [A. Key TB]                           |
       +---------------+-------------+------------+-------------+
    3  | S. Key Label  |  [S. Key TB]                           |
       +---------------+-------------+------------+-------------+
    4  | Cap A Cmd Lbl | [CapACmdTB] | CapASh lbl | [CapASh TB] |
       +---------------+-------------+------------+-------------+
    5  | Cap W Cmd Lbl | [CapWCmdTB] | CapWSh lbl | [CapWSh TB] |
       +---------------+-------------+------------+-------------+
    6  | CodeblkSh Lbl | [CodeblkSh TB]                         |
       +---------------+-------------+------------+-------------+
    7  |                               [] Show Welcome Screen   |
       +---------------+-------------+------------+-------------+
    8  | Test Conn Btn |  StatusLabel                           |
       +---------------+-------------+------------+-------------+
    9  | Vertical spacer                                        |
       +---------------+-------------+------------+-------------+
    10 | Dialog button Box{save, cancel}                        |
       +---------------+-------------+------------+-------------+
  */
  auto gridLayout = new QGridLayout(this);
  // row 0
  gridLayout->addWidget(new QLabel(tr("Evidence Repository"), this), 0, 0);
  gridLayout->addWidget(eviRepoTextBox, 0, 1, 1, 3);
  gridLayout->addWidget(eviRepoBrowseButton, 0, 4);

  // row 1
  gridLayout->addWidget(new QLabel(tr("Server URL"), this), 1, 0);
  gridLayout->addWidget(hostPathTextBox, 1, 1, 1, 4);

  // row 2
  gridLayout->addWidget(new QLabel(tr("Secret Key"), this), 2, 0);
  gridLayout->addWidget(secretKeyTextBox, 2, 1, 1, 4);

  // row 3
  gridLayout->addWidget(new QLabel(tr("Access Key"), this), 3, 0);
  gridLayout->addWidget(accessKeyTextBox, 3, 1, 1, 4);

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

  // row 7
  gridLayout->addWidget(showWelcomeScreen, 7, 1, 1, 5);

  // row 8
  gridLayout->addWidget(testConnectionButton, 8, 0);
  gridLayout->addWidget(connStatusLabel, 8, 1, 1, 4);

  // row 9
  gridLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Expanding), 9, 0, 1, gridLayout->columnCount());

  // row 10
  gridLayout->addWidget(buttonBox, 10, 0, 1, gridLayout->columnCount());

  setLayout(gridLayout);
  setSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);
  resize(760, 300);
  setWindowTitle(tr("Settings"));
}

void Settings::wireUi() {
  connect(NetMan::get(), &NetMan::testStatusChanged, this, &Settings::testStatusChanged);
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
  HotkeyManager::disableHotkeys();
  eviRepoTextBox->setFocus(); //setting focus to prevent retaining focus for macs

  // reset the form in case a user left junk in the text boxes and pressed "cancel"
  eviRepoTextBox->setText(QDir::toNativeSeparators(AppConfig::value(CONFIG::EVIDENCEREPO)));
  secretKeyTextBox->setText(AppConfig::value(CONFIG::SECRETKEY));
  accessKeyTextBox->setText(AppConfig::value(CONFIG::ACCESSKEY));
  hostPathTextBox->setText(AppConfig::value(CONFIG::APIURL));
  captureAreaCmdTextBox->setText(AppConfig::value(CONFIG::COMMAND_SCREENSHOT));
  captureAreaShortcutTextBox->setKeySequence(QKeySequence::fromString(AppConfig::value(CONFIG::SHORTCUT_SCREENSHOT)));
  captureWindowCmdTextBox->setText(AppConfig::value(CONFIG::COMMAND_CAPTUREWINDOW));
  captureWindowShortcutTextBox->setKeySequence(QKeySequence::fromString(AppConfig::value(CONFIG::SHORTCUT_CAPTUREWINDOW)));
  captureClipboardShortcutTextBox->setKeySequence(QKeySequence::fromString(AppConfig::value(CONFIG::SHORTCUT_CAPTURECLIPBOARD)));
  showWelcomeScreen->setChecked(AppConfig::value(CONFIG::SHOW_WELCOME_SCREEN) == "true");

  // re-enable form
  connStatusLabel->clear();
  testConnectionButton->setEnabled(true);
}

void Settings::closeEvent(QCloseEvent *event) {
  onSaveClicked();
  HotkeyManager::enableHotkeys();
  QDialog::closeEvent(event);
}

void Settings::onCancelClicked() {
  HotkeyManager::enableHotkeys();
  reject();
}

void Settings::onSaveClicked() {
  connStatusLabel->clear();

  AppConfig::setValue(CONFIG::EVIDENCEREPO, QDir::fromNativeSeparators(eviRepoTextBox->text()));
  AppConfig::setValue(CONFIG::ACCESSKEY, accessKeyTextBox->text());
  AppConfig::setValue(CONFIG::SECRETKEY, secretKeyTextBox->text());

  QString originalApiUrl = AppConfig::value(CONFIG::APIURL);
  AppConfig::setValue(CONFIG::APIURL, hostPathTextBox->text());
  if (originalApiUrl != hostPathTextBox->text())
    NetMan::refreshOperationsList();

  AppConfig::setValue(CONFIG::COMMAND_SCREENSHOT, captureAreaCmdTextBox->text());
  AppConfig::setValue(CONFIG::SHORTCUT_SCREENSHOT, captureAreaShortcutTextBox->keySequence().toString());
  AppConfig::setValue(CONFIG::COMMAND_CAPTUREWINDOW, captureWindowCmdTextBox->text());
  AppConfig::setValue(CONFIG::SHORTCUT_CAPTUREWINDOW, captureWindowShortcutTextBox->keySequence().toString());
  AppConfig::setValue(CONFIG::SHORTCUT_CAPTURECLIPBOARD, captureClipboardShortcutTextBox->keySequence().toString());
  QString showWelcome = showWelcomeScreen->isChecked() ? "true" : "false";
  AppConfig::setValue(CONFIG::SHOW_WELCOME_SCREEN, showWelcome);

  HotkeyManager::updateHotkeys();
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
    connStatusLabel->setText(tr("Please set Access Key, Secret key and Server URL first."));
    return;
  }
  NetMan::testConnection(hostPathTextBox->text(), accessKeyTextBox->text(), secretKeyTextBox->text());
}

void Settings::testStatusChanged(int result)
{
  if(result == NetMan::INPROGRESS) {
    testConnectionButton->startAnimation();
    testConnectionButton->setEnabled(false);
    connStatusLabel->setText("Testing");
    return;
  }
  testConnectionButton->stopAnimation();
  testConnectionButton->setEnabled(true);
  connStatusLabel->setText(NetMan::lastTestError());
}
