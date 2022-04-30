#include "porting_dialog.h"

#include <QFileDialog>
#include <QtConcurrent/QtConcurrent>
#include <QMessageBox>
#include <iostream>

PortingDialog::PortingDialog(PortType dialogType, DatabaseConnection* db, QWidget *parent)
  : QDialog(parent) {
  this->dialogType = dialogType;
  this->db = db;

  buildUi();
  wireUi();
}

PortingDialog::~PortingDialog() {
  delete closeWindowAction;

  delete _selectFileLabel;
  delete portConfigCheckBox;
  delete portEvidenceCheckBox;
  delete submitButton;
  delete pathTextBox;
  delete progressBar;
  delete portStatusLabel;

  delete gridLayout;
}

void PortingDialog::buildUi() {
  gridLayout = new QGridLayout(this);

  _selectFileLabel = new QLabel(this);
  portStatusLabel = new QLabel(this);

  submitButton = new QPushButton(this);
  browseButton = new QPushButton(tr("Browse"), this);
  pathTextBox = new QLineEdit(this);
  portConfigCheckBox = new QCheckBox(tr("Include Config"), this);
  portConfigCheckBox->setChecked(true);
  portEvidenceCheckBox = new QCheckBox(tr("Include Evidence"), this);
  portEvidenceCheckBox->setChecked(true);
  progressBar = new QProgressBar(this);


  if (dialogType == Import) {
    submitButton->setText(tr("Import"));
    _selectFileLabel->setText(tr("Select import file"));
    this->setWindowTitle(tr("Import Data"));
  }
  else {
    submitButton->setText(tr("Export"));
    _selectFileLabel->setText(tr("Export Directory"));
    this->setWindowTitle(tr("Export Data"));
  }

  // Layout
  /*        0                 1           2
       +---------------+-------------+--------------+
    0  | File  Lbl     | [path TB]   | Browse Btn   |
       +---------------+-------------+--------------+
    1  |                 With Cfg CB                |
       +---------------+-------------+--------------+
    2  |                 With data CB               |
       +---------------+-------------+--------------+
    3  |                 Progress Bar               |
       +---------------+-------------+--------------+
    4  |                Porting Status              |
       +---------------+-------------+--------------+
    5  | <None>        | <None>      | Submit Btn   |
       +---------------+-------------+--------------+
  */

  // row 0
  gridLayout->addWidget(_selectFileLabel, 0, 0);
  gridLayout->addWidget(pathTextBox, 0, 1);
  gridLayout->addWidget(browseButton, 0, 2);

  // row 1
  gridLayout->addWidget(portConfigCheckBox, 1, 0, 1, gridLayout->columnCount());

  // row 2
  gridLayout->addWidget(portEvidenceCheckBox, 2, 0, 1, gridLayout->columnCount());

  // row 3
  gridLayout->addWidget(progressBar, 3, 0, 1, gridLayout->columnCount());

  // row 4
  gridLayout->addWidget(portStatusLabel, 4, 0, 1, gridLayout->columnCount());

  // row 5
  gridLayout->addWidget(submitButton, 5, 2);

  closeWindowAction = new QAction(this);
  closeWindowAction->setShortcut(QKeySequence::Close);
  this->addAction(closeWindowAction);

  this->setLayout(gridLayout);
  this->resize(500, 1);
}

void PortingDialog::wireUi() {
  auto btnClicked = &QPushButton::clicked;

  connect(submitButton, btnClicked, this, &PortingDialog::onSubmitPressed);
  connect(browseButton, btnClicked, this, &PortingDialog::onBrowsePresed);
}

void PortingDialog::onBrowsePresed() {
  auto browseStart = pathTextBox->text();
  browseStart = QFile(browseStart).exists() ? browseStart : QDir::homePath();
  QString selectedFile;
  if (dialogType == Import) {
    selectedFile = QFileDialog::getOpenFileName(this, tr("Select an import file"),
                                                browseStart, tr("System Migration json (system.json);;All Files(*)"));
  }
  else {
    selectedFile = QFileDialog::getExistingDirectory(this, tr("Select an export directory"),
                                                     browseStart, QFileDialog::ShowDirsOnly);
  }
  if (!selectedFile.isNull()) {
    pathTextBox->setText(QDir::toNativeSeparators(selectedFile));
  }
}

void PortingDialog::resetForm() {
    portDone = false;
    pathTextBox->clear();
    progressBar->setRange(0, 1);
    progressBar->setValue(0);
    portStatusLabel->clear();
    portConfigCheckBox->setCheckState(Qt::Unchecked);
    portEvidenceCheckBox->setCheckState(Qt::Unchecked);
    submitButton->setText(dialogType == Import ? tr("Import") : tr("Export"));
}

void PortingDialog::onSubmitPressed() {
  if (portDone) {
      this->close();
      resetForm();
  }
  auto portingPath = pathTextBox->text().trimmed();
  if (pathTextBox->text().trimmed().isEmpty()) {
    portStatusLabel->setText(tr("Please set a valid path first"));
    return;
  }

  submitButton->setEnabled(false);
  progressBar->setRange(0, 0);

  std::function<void()> portAction;

  portStatusLabel->setText(tr("Gathering data..."));

  if (dialogType == Import) {
    executedManifest = doPreImport(portingPath);
    if (executedManifest == nullptr) {
      return;
    }
    portAction = [this](){ doImport(executedManifest); };
    portPath = FileHelpers::getDirname(portingPath);
  }
  else {
    executedManifest = new porting::SystemManifest();
    portAction = [this, portingPath](){ doExport(executedManifest, portingPath); };
    portPath = portingPath;
  }

  connect(executedManifest, &porting::SystemManifest::onReady, progressBar, &QProgressBar::setMaximum);
  connect(executedManifest, &porting::SystemManifest::onFileProcessed, progressBar, &QProgressBar::setValue);
  connect(executedManifest, &porting::SystemManifest::onStatusUpdate, portStatusLabel, &QLabel::setText);
  connect(this, &PortingDialog::onWorkComplete, this, &PortingDialog::onPortComplete);

  std::ignore = QtConcurrent::run(portAction);
}

void PortingDialog::onPortComplete(bool success) {
  if(success) {
    portStatusLabel->setText(dialogType == Import ? tr("Import Complete") : tr("Export Complete"));
    submitButton->setText(tr("Close"));
    portDone = true;
  }
  if(executedManifest != nullptr) {
    delete executedManifest;
    executedManifest = nullptr;
  }
  progressBar->setRange(0, 1);
  progressBar->setValue(1);
  submitButton->setEnabled(true);
  Q_EMIT portCompleted(portPath);
}

QString PortingDialog::getPortPath() {
  return portPath;
}

void PortingDialog::doExport(porting::SystemManifest* manifest, const QString& exportPath) {
  porting::SystemManifestExportOptions options;
  options.exportDb = portEvidenceCheckBox->isChecked();
  options.exportConfig = portConfigCheckBox->isChecked();

  // Qt db access is limited to single-thread access. A new connection needs to be made, hence
  // the withconnection here that connects to the same database. Note: we shouldn't write to the db
  // in this thread, if possible.
  QString threadedDbName = Constants::defaultDbName() + "_mt_forExport";
  DatabaseConnection::withConnection(
      db->getDatabasePath(), threadedDbName, [this, &manifest, exportPath, options](DatabaseConnection conn){
         try {
           manifest->exportManifest(&conn, exportPath, options);
         }
         catch(const FileError &e) {
           portStatusLabel->setText(tr("Error during export: %1").arg(e.what()));
           Q_EMIT onWorkComplete(false);
         }
         catch(const QSqlError &e) {
           portStatusLabel->setText(tr("Error during export: %1").arg(e.text()));
           Q_EMIT onWorkComplete(false);
         }
  });
  Q_EMIT onWorkComplete(true);
}

porting::SystemManifest* PortingDialog::doPreImport(const QString& pathToSystemManifest) {
  porting::SystemManifest* manifest = nullptr;
  try {
    manifest = porting::SystemManifest::readManifest(pathToSystemManifest);
  }
  catch(const FileError& e) {
    portStatusLabel->setText(tr("Unable to parse system file."));
    onPortComplete(false);
  }
  return manifest;
}

void PortingDialog::doImport(porting::SystemManifest* manifest) {
  porting::SystemManifestImportOptions options;
  options.importDb = portEvidenceCheckBox->isChecked() ? options.Merge : options.None;
  options.importConfig = portConfigCheckBox->isChecked();

  QString threadedDbName = QStringLiteral("%1_mt_forImport").arg(Constants::defaultDbName());
  DatabaseConnection::withConnection(
      db->getDatabasePath(), threadedDbName, [this, &manifest, options](DatabaseConnection conn){
        try {
          manifest->applyManifest(options, &conn);
        }
        catch(const FileError &e) {
          portStatusLabel->setText(tr("Error during import: %1").arg(e.what()));
          Q_EMIT onWorkComplete(false);
        }
        catch(const QSqlError &e) {
          portStatusLabel->setText(tr("Error during import: ").arg(e.text()));
          Q_EMIT onWorkComplete(false);
        }
  });
  Q_EMIT onWorkComplete(true);
}
