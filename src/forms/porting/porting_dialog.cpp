#include "porting_dialog.h"

#include <QFileDialog>
#include <QtConcurrent/QtConcurrent>
#include <iostream>

#include "porting/system_manifest.h"

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
  browseButton = new QPushButton("Browse", this);
  pathTextBox = new QLineEdit(this);
  portConfigCheckBox = new QCheckBox("Include Config", this);
  portConfigCheckBox->setChecked(true);
  portEvidenceCheckBox = new QCheckBox("Include Evidence", this);
  portEvidenceCheckBox->setChecked(true);
  progressBar = new QProgressBar(this);
  // set some initial values to provide a
  progressBar->setRange(0, 1);
  progressBar->setValue(0);


  if (dialogType == Import) {
    submitButton->setText("Import");
    _selectFileLabel->setText("Select import file");
    this->setWindowTitle("Import Data");
  }
  else {
    submitButton->setText("Export");
    _selectFileLabel->setText("Export Directory");
    this->setWindowTitle("Export Data");
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

void PortingDialog::onSubmitPressed() {
  if (pathTextBox->text().trimmed().isEmpty()) {
    portStatusLabel->setText("Please set a valid path first");
    return;
  }

  submitButton->setEnabled(false);
  progressBar->setRange(0, 0);

  if (dialogType == Import) {
    portStatusLabel->setText("Parsing input...");
    QtConcurrent::run([this](){doImport();});
  }
  else {
    portStatusLabel->setText("Gathering data...");
    QtConcurrent::run([this](){doExport();});
  }
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

void PortingDialog::onPortComplete(bool success) {
  submitButton->setEnabled(true);
}

void PortingDialog::doExport() {
  sync::SystemManifestExportOptions options;
  options.exportDb = portEvidenceCheckBox->isChecked();
  options.exportConfig = portConfigCheckBox->isChecked();
  sync::SystemManifest manifest;
  connect(&manifest, &sync::SystemManifest::onReady, progressBar, &QProgressBar::setMaximum);
  connect(&manifest, &sync::SystemManifest::onFileProcessed, progressBar, &QProgressBar::setValue);
  connect(&manifest, &sync::SystemManifest::onStatusUpdate, portStatusLabel, &QLabel::setText);

  // Qt db access is limited to single-thread access. A new connection needs to be made, hence
  // the withconnection here that connects to the same database. Note: we shouldn't write to the db
  // in this thread, if possible.
  QString threadedDbName = Constants::defaultDbName() + "_mt_forExport";
  DatabaseConnection::withConnection(
      db->getDatabasePath(), threadedDbName, [this, &manifest, options](DatabaseConnection conn){
         try {
           manifest.exportManifest(&conn, pathTextBox->text(), options);
           portStatusLabel->setText("Export Complete");
           onPortComplete(true);
         }
         catch(const FileError &e) {
           std::cout << "got an error during export: " << e.what() << std::endl;
           onPortComplete(false);
         }
         catch(const QSqlError &e) {
           std::cout << "got an sql error during export: " << e.text().toStdString() << std::endl;
           onPortComplete(false);
         }
  });
}

void PortingDialog::doImport() {
  sync::SystemManifestImportOptions options;
  options.importDb = portEvidenceCheckBox->isChecked() ? options.Merge : options.None;
  options.importConfig = portConfigCheckBox->isChecked();
  sync::SystemManifest* manifest;
  try {
    manifest = sync::SystemManifest::readManifest(pathTextBox->text());
  }
  catch(const FileError& e) {
    portStatusLabel->setText("Unable to parse system file.");
    onPortComplete(false);
    return;
  }

  connect(manifest, &sync::SystemManifest::onReady, progressBar, &QProgressBar::setMaximum);
  connect(manifest, &sync::SystemManifest::onFileProcessed, progressBar, &QProgressBar::setValue);
  connect(manifest, &sync::SystemManifest::onStatusUpdate, portStatusLabel, &QLabel::setText);

  QString threadedDbName = Constants::defaultDbName() + "_mt_forImport";
  DatabaseConnection::withConnection(
      db->getDatabasePath(), threadedDbName, [this, &manifest, options](DatabaseConnection conn){
        try {
          manifest->applyManifest(options, &conn);
          portStatusLabel->setText("Import Complete");
          delete manifest;
        }
        catch(const FileError &e) {
          portStatusLabel->setText(QString("Error during import: ") + e.what());
          std::cout << "got an error during import: " << e.what() << std::endl;
          onPortComplete(false);
        }
        catch(const QSqlError &e) {
          portStatusLabel->setText(QString("Error during import: ") + e.text());
          std::cout << "got an sql error during import: " << e.text().toStdString() << std::endl;
          onPortComplete(false);
        }
  });
  onPortComplete(true);
}
