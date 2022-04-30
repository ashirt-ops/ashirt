// Copyright 2020, Verizon Media
// Licensed under the terms of MIT. See LICENSE file in project root for terms.

#include "evidencemanager.h"

#include <QCheckBox>
#include <QHeaderView>
#include <QKeySequence>
#include <QMessageBox>
#include <QRandomGenerator>
#include <QStandardPaths>
#include <QTableWidgetItem>
#include <iostream>

#include "appsettings.h"
#include "dtos/tag.h"
#include "forms/evidence_filter/evidencefilter.h"
#include "forms/evidence_filter/evidencefilterform.h"
#include "helpers/clipboard/clipboardhelper.h"
#include "helpers/netman.h"
#include "helpers/stopreply.h"

enum ColumnIndexes {
  COL_DATE_CAPTURED = 0,
  COL_OPERATION,
  COL_PATH,
  COL_CONTENT_TYPE,
  COL_DESCRIPTION,
  COL_SUBMITTED,
  COL_DATE_SUBMITTED,
  COL_FAILED,
  COL_ERROR_MSG
};

static QStringList columnNames() {
  static QStringList names;
  if (names.count() == 0) {
    names.insert(COL_DATE_CAPTURED, QStringLiteral("Date Captured"));
    names.insert(COL_OPERATION, QStringLiteral("Operation"));
    names.insert(COL_PATH, QStringLiteral("Path"));
    names.insert(COL_CONTENT_TYPE, QStringLiteral("Content Type"));
    names.insert(COL_DESCRIPTION, QStringLiteral("Description"));
    names.insert(COL_SUBMITTED, QStringLiteral("Submitted"));
    names.insert(COL_DATE_SUBMITTED, QStringLiteral("Date Submitted"));
    names.insert(COL_FAILED, QStringLiteral("Failed"));
    names.insert(COL_ERROR_MSG, QStringLiteral("Error"));
  }
  return names;
}

EvidenceManager::EvidenceManager(DatabaseConnection* db, QWidget* parent) : QDialog(parent) {
  this->db = db;
  buildUi();
  wireUi();
}

EvidenceManager::~EvidenceManager() {
  delete submitEvidenceAction;
  delete deleteEvidenceAction;
  delete copyPathToClipboardAction;
  delete deleteTableContentsAction;
  delete closeWindowAction;
  delete evidenceTableContextMenu;
  delete filterForm;
  delete evidenceEditor;
  delete editButton;
  delete cancelEditButton;
  delete editFiltersButton;
  delete applyFilterButton;
  delete resetFilterButton;
  delete filterTextBox;
  delete evidenceTable;
  delete loadingAnimation;

  delete gridLayout;
  stopReply(&uploadAssetReply);
}

void EvidenceManager::buildEvidenceTableUi() {
  evidenceTable = new QTableWidget(this);
  evidenceTable->setContextMenuPolicy(Qt::CustomContextMenu);
  QStringList colNames = columnNames();
  evidenceTable->setColumnCount(colNames.length());
  evidenceTable->setHorizontalHeaderLabels(colNames);
  evidenceTable->setSelectionMode(QAbstractItemView::SingleSelection);
  evidenceTable->setSelectionBehavior(QAbstractItemView::SelectRows);
  evidenceTable->setSortingEnabled(true);
  evidenceTable->verticalHeader()->setVisible(false);
  evidenceTable->horizontalHeader()->setCascadingSectionResizes(false);
  evidenceTable->horizontalHeader()->setStretchLastSection(true);
  evidenceTable->horizontalHeader()->setSortIndicatorShown(true);
  evidenceTable->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  evidenceTable->setSelectionMode(QAbstractItemView::SelectionMode::ExtendedSelection);
}

void EvidenceManager::buildUi() {
  gridLayout = new QGridLayout(this);
  filterForm = new EvidenceFilterForm(this);
  evidenceTableContextMenu = new QMenu(this);
  submitEvidenceAction = new QAction(tr("Submit Evidence"), evidenceTableContextMenu);
  evidenceTableContextMenu->addAction(submitEvidenceAction);
  deleteEvidenceAction = new QAction(tr("Delete Evidence"), evidenceTableContextMenu);
  evidenceTableContextMenu->addAction(deleteEvidenceAction);
  copyPathToClipboardAction = new QAction(tr("Copy Path"), evidenceTableContextMenu);
  evidenceTableContextMenu->addAction(copyPathToClipboardAction);
  evidenceTableContextMenu->addSeparator();
  deleteTableContentsAction = new QAction(tr("Delete All from table"), evidenceTableContextMenu);
  evidenceTableContextMenu->addAction(deleteTableContentsAction);

  filterTextBox = new QLineEdit(this);
  editFiltersButton = new QPushButton(tr("Edit Filters"), this);
  applyFilterButton = new QPushButton(tr("Apply"), this);
  resetFilterButton = new QPushButton(tr("Reset"), this);

  editButton = new QPushButton(tr("Edit"), this);
  cancelEditButton = new QPushButton(tr("Cancel"), this);
  cancelEditButton->setVisible(false);

  // remove button defaults (i.e. enter-submits-form functionality)
  editFiltersButton->setAutoDefault(false);
  applyFilterButton->setAutoDefault(false);
  resetFilterButton->setAutoDefault(false);
  editButton->setAutoDefault(false);
  cancelEditButton->setAutoDefault(false);

  // apply a default for apply-filter, which is the typical action
  applyFilterButton->setDefault(true);

  buildEvidenceTableUi();

  evidenceEditor = new EvidenceEditor(db, this);
  evidenceEditor->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));

  loadingAnimation = new QProgressIndicator(this);
  spacer = new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Minimum);

  setTabOrder(editFiltersButton, filterTextBox);
  setTabOrder(filterTextBox, applyFilterButton);
  setTabOrder(applyFilterButton, resetFilterButton);
  setTabOrder(resetFilterButton, evidenceTable);

  // Layout
  /*        0                 1           2             3
       +---------------+-------------+------------+-------------+
    0  | EditFilt Btn  | [Filt TB]   | Apply Btn  | Reset Btn   |
       +---------------+-------------+------------+-------------+
    1  |                                                        |
       |                     Evidence Table                     |
       |                                                        |
       +---------------+-------------+------------+-------------+
    2  |                                                        |
       |                     Evidence Editor                    |
       |                                                        |
       +---------------+-------------+------------+-------------+
    3  | Loading Ani   | Hor Spacer  | Cancel Btn | Edit Btn    |
       +---------------+-------------+------------+-------------+
  */

  // row 0
  gridLayout->addWidget(editFiltersButton, 0, 0);
  gridLayout->addWidget(filterTextBox, 0, 1);
  gridLayout->addWidget(applyFilterButton, 0, 2);
  gridLayout->addWidget(resetFilterButton, 0, 3);

  // row 1
  gridLayout->addWidget(evidenceTable, 1, 0, 1, gridLayout->columnCount());

  // row 2
  gridLayout->addWidget(evidenceEditor, 2, 0, 1, gridLayout->columnCount());

  // row 3
  gridLayout->addWidget(loadingAnimation, 3, 0);
  gridLayout->addItem(spacer, 3, 1);
  gridLayout->addWidget(cancelEditButton, 3, 2);
  gridLayout->addWidget(editButton, 3, 3);

  closeWindowAction = new QAction(this);
  closeWindowAction->setShortcut(QKeySequence::Close);
  this->addAction(closeWindowAction);

  this->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);
  this->resize(800, 600);
  this->setWindowTitle(tr("Evidence Manager"));
  this->setLayout(gridLayout);
}

void EvidenceManager::wireUi() {
  // remap events to shorten the connect lines
  auto btnClicked = &QPushButton::clicked;
  auto actionTriggered = &QAction::triggered;

  connect(applyFilterButton, btnClicked, this, &EvidenceManager::loadEvidence);
  connect(resetFilterButton, btnClicked, this, &EvidenceManager::resetFilterButtonClicked);
  connect(editFiltersButton, btnClicked, this, &EvidenceManager::openFiltersMenu);
  connect(editButton, btnClicked, this, &EvidenceManager::editEvidenceButtonClicked);
  connect(cancelEditButton, btnClicked, this, &EvidenceManager::cancelEditEvidenceButtonClicked);

  connect(submitEvidenceAction, actionTriggered, this, &EvidenceManager::submitEvidenceTriggered);
  connect(deleteEvidenceAction, actionTriggered, this, &EvidenceManager::deleteEvidenceTriggered);
  connect(closeWindowAction, actionTriggered, this, &EvidenceManager::close);
  connect(copyPathToClipboardAction, actionTriggered, this, &EvidenceManager::copyPathTriggered);
  connect(deleteTableContentsAction, actionTriggered, this, &EvidenceManager::deleteAllTriggered);

  connect(filterForm, &EvidenceFilterForm::evidenceSet, this, &EvidenceManager::applyFilterForm);

  connect(this, &EvidenceManager::evidenceChanged, evidenceEditor, &EvidenceEditor::updateEvidence);
  connect(evidenceTable, &QTableWidget::currentCellChanged, this, &EvidenceManager::onRowChanged);
  connect(evidenceTable, &QTableWidget::customContextMenuRequested, this,
          &EvidenceManager::openTableContextMenu);
}

void EvidenceManager::editEvidenceButtonClicked() {
  if(editButton->text() == tr("Save")) {
    evidenceEditor->saveEvidence();
    cancelEditEvidenceButtonClicked();
    refreshRow(evidenceTable->currentRow());
    // restore default form action
    applyFilterButton->setDefault(true);
  }
  else {
    // remove default form action to prevent accidental reloading of evidence
    applyFilterButton->setDefault(false);
    evidenceEditor->setEnabled(true);
    editButton->setText(tr("Save"));
    cancelEditButton->setVisible(true);
  }
}

void EvidenceManager::cancelEditEvidenceButtonClicked() {
  evidenceEditor->setEnabled(false);
  cancelEditButton->setVisible(false);
  //refreshRow(evidenceTable->currentRow());
  editButton->setText(tr("Edit"));
  evidenceEditor->revert();
}

void EvidenceManager::showEvent(QShowEvent* evt) {
  QDialog::showEvent(evt);
  evidenceEditor->updateEvidence(-1, true);
  resetFilterButtonClicked();
}

void EvidenceManager::submitEvidenceTriggered() {
  loadingAnimation->startAnimation();
  evidenceTable->setEnabled(false);  // prevent switching evidence while one is being submitted.
  if (saveData()) {
    evidenceIDForRequest = selectedRowEvidenceID();
    try {
      model::Evidence evi = db->getEvidenceDetails(evidenceIDForRequest);
      uploadAssetReply = NetMan::getInstance().uploadAsset(evi);
      connect(uploadAssetReply, &QNetworkReply::finished, this, &EvidenceManager::onUploadComplete);
    }
    catch (QSqlError& e) {
      evidenceTable->setEnabled(true);
      loadingAnimation->stopAnimation();
      QMessageBox::warning(this, tr("Cannot submit evidence"),
                           tr("Could not retrieve data. Please try again."));
    }
  }
}

void EvidenceManager::deleteEvidenceTriggered() {
  std::vector<qint64> ids = selectedRowEvidenceIDs();
  QString thisMuch = ids.size() > 1 ? QString("these %1 pieces of").arg(ids.size()) : "this";
  auto reply = QMessageBox::question(this, tr("Discard Evidence"),
                            tr("Are you sure you want to discard %1 "
                               " evidence? This will only delete this evidence on your computer.").arg(thisMuch),
                            QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

  if (reply == QMessageBox::Yes) {
    deleteSet(ids);
  }
}

void EvidenceManager::deleteAllTriggered() {
  auto reply = QMessageBox::question(this, tr("Delete All Evidence"), tr("Warning: This will delete ALL "
                                     "evidence currently listed in this table. Do you want to continue?"),
                                     QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

  if (reply == QMessageBox::Yes) {
    std::vector<qint64> ids;
    for(int rowIndex = 0; rowIndex < evidenceTable->rowCount(); rowIndex++) {
      ids.push_back(evidenceTable->item(rowIndex, 0)->data(Qt::UserRole).toLongLong());
    }
    deleteSet(ids);
  }
}

/// parentDir returns the parent directory for a given file path.
/// example: Input: path/to/file.txt Output: path/to
static QString parentDir(QString path) {
  auto lastSlash = path.lastIndexOf("/");
  return path.left(lastSlash);
}

void EvidenceManager::deleteSet(std::vector<qint64> ids) {
  std::vector<DeleteEvidenceResponse> responses = evidenceEditor->deleteEvidence(ids);
  QStringList undeletedFiles;
  bool removedAllDbRecords = true;
  QStringList paths;
  for(const auto& response : responses) {
    if (!response.fileDeleteSuccess) {
      undeletedFiles << response.model.path;
    }
    removedAllDbRecords = removedAllDbRecords && response.dbDeleteSuccess;
    auto parentPath = parentDir(response.model.path);
    if (!paths.contains(parentPath)) {
      paths << parentPath;
    }
  }

  if (!removedAllDbRecords) {
    std::cerr << "Could not delete evidence from internal database. Errors: " << std::endl;
    for (const auto& resp : responses) {
      if (!resp.dbDeleteSuccess) {
        std::cerr << "  id: " << resp.model.id << " ;; Error: "<< resp.errorText.toStdString() << std::endl;
      }
    }
  }

  if (undeletedFiles.length() > 0) {
    bool logWritten = true;
    auto today = QDateTime::currentDateTime();
    auto errLogPath = AppConfig::getInstance().evidenceRepo + "/" +QString("%1.log").arg(today.toMSecsSinceEpoch());
    try {
      FileHelpers::writeFile(errLogPath,
                             tr("Paths to files that could not be deleted: \n\n %1")
                               .arg(undeletedFiles.join(QStringLiteral("\n"))));
    }
    catch(FileError &e) {
      logWritten = false;
    }
    QString msg = tr("Some files could not be deleted.");

    if (logWritten) {
      msg.append(tr(" A list of the excluded files can be found here: \n").arg(errLogPath));
    }
    QMessageBox::warning(this, tr("Could not complete evidence deletion"), msg);
  }

  for (const auto& p : paths) {
    auto path = QDir(p);
    auto dirName = path.dirName();
    path.cdUp();
    path.rmdir(dirName);
  }

  loadEvidence();
}

void EvidenceManager::copyPathTriggered() {
  auto evidence = db->getEvidenceDetails(selectedRowEvidenceID());
  ClipboardHelper::setText(evidence.path);
}

void EvidenceManager::openTableContextMenu(QPoint pos) {
  int selectedRowCount = evidenceTable->selectionModel()->selectedRows().count();
  if (selectedRowCount == 0) {
    return;
  }
  bool singleItemSelected = selectedRowCount == 1;
  copyPathToClipboardAction->setEnabled(singleItemSelected);
  bool wasSubmitted = !evidenceEditor->encodeEvidence().uploadDate.isNull();
  submitEvidenceAction->setEnabled(singleItemSelected && !wasSubmitted);

  evidenceTableContextMenu->popup(evidenceTable->viewport()->mapToGlobal(pos));
}

void EvidenceManager::resetFilterButtonClicked() {
  EvidenceFilters filter;
  filter.operationSlug = AppSettings::getInstance().operationSlug();
  filterTextBox->setText(filter.toString());
  loadEvidence();
}

void EvidenceManager::applyFilterForm(const EvidenceFilters& filter) {
  filterTextBox->setText(filter.toString());
  loadEvidence();
}

void EvidenceManager::loadEvidence() {
  qint64 reselectId = -1;
  if (evidenceTable->selectedItems().size() > 0) {
    reselectId = selectedRowEvidenceID();
  }

  evidenceTable->clearContents();

  try {
    auto filter = EvidenceFilters::parseFilter(filterTextBox->text());
    std::vector<model::Evidence> operationEvidence = db->getEvidenceWithFilters(filter);
    evidenceTable->setRowCount(operationEvidence.size());

    // removing sorting temporarily to solve a bug (per qt: not a bug)
    // Essentially, _not_ doing this breaks reloading the table. Mostly empty cells appear.
    // from: https://stackoverflow.com/a/8904287/4262552
    // see also: https://bugreports.qt.io/browse/QTBUG-75479
    evidenceTable->setSortingEnabled(false);
    for (size_t row = 0; row < operationEvidence.size(); row++) {
      auto evi = operationEvidence.at(row);
      auto rowData = buildBaseEvidenceRow(evi.id);

      evidenceTable->setItem(row, COL_OPERATION, rowData.operation);
      evidenceTable->setItem(row, COL_DESCRIPTION, rowData.description);
      evidenceTable->setItem(row, COL_CONTENT_TYPE, rowData.contentType);
      evidenceTable->setItem(row, COL_DATE_CAPTURED, rowData.dateCaptured);
      evidenceTable->setItem(row, COL_PATH, rowData.path);
      evidenceTable->setItem(row, COL_FAILED, rowData.failed);
      evidenceTable->setItem(row, COL_ERROR_MSG, rowData.errorText);
      evidenceTable->setItem(row, COL_SUBMITTED, rowData.submitted);
      evidenceTable->setItem(row, COL_DATE_SUBMITTED, rowData.dateSubmitted);

      setRowText(row, evi);
    }
    evidenceTable->setSortingEnabled(true);
    if (evidenceTable->rowCount() > 0) {
      // try to reselect the last viewed evidence, if it's still in the list
      int selectRow = 0;
      for (int rowIndex = 0; rowIndex < evidenceTable->rowCount(); rowIndex++) {
        auto evidenceID = evidenceTable->item(rowIndex, 0)->data(Qt::UserRole).toLongLong();
        if(evidenceID == reselectId) {
          selectRow = rowIndex;
          break;
        }
      }
      evidenceTable->setCurrentCell(selectRow, 0);
    }
  }
  catch (QSqlError& e) {
    std::cout << "Could not retrieve evidence for operation. Error: " << e.text().toStdString()
              << std::endl;
  }
}

// buildBaseEvidenceRow constructs a container for a row of data.
// Note: the row (container) is on the stack, but items in the container
// are on the heap, and must be deleted.
EvidenceRow EvidenceManager::buildBaseEvidenceRow(qint64 evidenceID) {
  EvidenceRow row{};
  auto basicItem = [evidenceID]() -> QTableWidgetItem* {
    auto item = new QTableWidgetItem();
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    item->setData(Qt::UserRole, evidenceID);
    return item;
  };

  row.dateCaptured = basicItem();
  row.description = basicItem();
  row.operation = basicItem();
  row.contentType = basicItem();
  row.path = basicItem();
  row.errorText = basicItem();
  row.dateSubmitted = basicItem();

  row.submitted = basicItem();
  row.submitted->setTextAlignment(Qt::AlignCenter);
  row.failed = basicItem();
  row.failed->setTextAlignment(Qt::AlignCenter);

  return row;
}

void EvidenceManager::setRowText(int row, const model::Evidence& model) {
  static QString dateFormat = QLocale().dateTimeFormat(QLocale::ShortFormat);

  auto setColText = [this, row](int col, QString text) {
    evidenceTable->item(row, col)->setText(text);
  };
  setColText(COL_DATE_CAPTURED, model.recordedDate.toLocalTime().toString(dateFormat));
  setColText(COL_DESCRIPTION, model.description);
  setColText(COL_OPERATION, model.operationSlug);
  setColText(COL_CONTENT_TYPE, model.contentType);
  setColText(COL_SUBMITTED, model.uploadDate.isNull() ? QStringLiteral("No") : QStringLiteral("Yes"));
  setColText(COL_FAILED, model.errorText.isEmpty() ? QString() : QStringLiteral("Yes"));
  setColText(COL_PATH, QDir::toNativeSeparators(model.path));
  setColText(COL_ERROR_MSG, model.errorText);

  auto uploadDateText = model.uploadDate.isNull() ? QStringLiteral("Never") : model.uploadDate.toLocalTime().toString(dateFormat);
  setColText(COL_DATE_SUBMITTED, uploadDateText);
}

void EvidenceManager::refreshRow(int row) {
  auto evidenceID = selectedRowEvidenceID();
  try {
    auto updatedData = db->getEvidenceDetails(evidenceID);
    setRowText(row, updatedData);
  }
  catch (QSqlError& e) {
    std::cout << "Could not refresh table row: " << e.text().toStdString() << std::endl;
  }
}

bool EvidenceManager::saveData() {
  auto saveResponse = evidenceEditor->saveEvidence();
  if (saveResponse.actionSucceeded) {
    refreshRow(evidenceTable->currentRow());
    return true;
  }

  QMessageBox::warning(this, tr("Cannot Save"),
                       tr("Unable to save evidence data.\n"
                       "You can try uploading directly to the website. File Location:\n%1")
                        .arg(saveResponse.model.path));
  return false;
}

void EvidenceManager::openFiltersMenu() {
  filterForm->setForm(EvidenceFilters::parseFilter(filterTextBox->text()));
  filterForm->open();
}

void EvidenceManager::onRowChanged(int currentRow, int _currentColumn, int _previousRow,
                                   int _previousColumn) {
  Q_UNUSED(_currentColumn);
  Q_UNUSED(_previousRow);
  Q_UNUSED(_previousColumn);

  cancelEditEvidenceButtonClicked();
  if (currentRow == -1) {
    editButton->setEnabled(false);
    editButton->setToolTip(tr("You must have some evidence selected to edit"));
    Q_EMIT evidenceChanged(-1, true);
    return;
  }

  auto evidence = db->getEvidenceDetails(selectedRowEvidenceID());

  auto readonly = evidence.uploadDate.isValid();
  submitEvidenceAction->setEnabled(!readonly);
  Q_EMIT evidenceChanged(evidence.id, true);

  int selectedRowCount = evidenceTable->selectionModel()->selectedRows().count();
  if (selectedRowCount > 1) {
    editButton->setEnabled(false);
    editButton->setToolTip(tr("Only one evidence item may be edited at once."));
  }
  else {
    this->editButton->setEnabled(!readonly);
    this->editButton->setToolTip(readonly
                                     ? tr("Edit is only available on unsubmitted evidence")
                                     : tr("Update this data before submitting"));
  }
}

void EvidenceManager::onUploadComplete() {
  bool isValid;
  NetMan::extractResponse(uploadAssetReply, isValid);

  if (!isValid) {
    auto errMessage =
        tr("Unable to upload evidence: Network error (%1)").arg(uploadAssetReply->errorString());
    try {
      db->updateEvidenceError(errMessage, evidenceIDForRequest);
    }
    catch (QSqlError& e) {
      std::cout << "Upload failed. Could not update internal database. Error: "
                << e.text().toStdString() << std::endl;
    }
    QMessageBox::warning(this, tr("Cannot Submit Evidence"),
                         tr("Upload failed: Network error. Check your connection and try again.\n"
                         "(Error: %1)").arg(uploadAssetReply->errorString()));
  }
  else {
    try {
      db->updateEvidenceSubmitted(evidenceIDForRequest);
    }
    catch (QSqlError& e) {
      std::cout << "Upload successful. Could not update internal database. Error: "
                << e.text().toStdString() << std::endl;
    }
    Q_EMIT evidenceChanged(evidenceIDForRequest, true);  // lock the editing form
  }
  refreshRow(evidenceTable->currentRow());

  // we don't actually need anything from the uploadAssets reply, so just clean it up.
  // one thing we might want to record: evidence uuid... not sure why we'd need it though.
  loadingAnimation->stopAnimation();
  evidenceTable->setEnabled(true);

  tidyReply(&uploadAssetReply);
}

qint64 EvidenceManager::selectedRowEvidenceID() {
  return evidenceTable->currentItem()->data(Qt::UserRole).toLongLong();
}

std::vector<qint64> EvidenceManager::selectedRowEvidenceIDs() {
  std::vector<qint64> rtn;

  // relies on the fact that entire rows are selected
  auto itemList = evidenceTable->selectionModel()->selectedRows();
  for (auto item : itemList) {
    rtn.push_back(item.data(Qt::UserRole).toLongLong());
  }
  return  rtn;
}
