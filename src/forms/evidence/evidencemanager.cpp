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
    names.insert(COL_ERROR_MSG, "Error");
    names.insert(COL_DATE_CAPTURED, "Date Captured");
    names.insert(COL_OPERATION, "Operation");
    names.insert(COL_PATH, "Path");
    names.insert(COL_CONTENT_TYPE, "Content Type");
    names.insert(COL_DESCRIPTION, "Description");
    names.insert(COL_SUBMITTED, "Submitted");
    names.insert(COL_DATE_SUBMITTED, "Date Submitted");
    names.insert(COL_FAILED, "Failed");
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
  delete closeWindowAction;
  delete evidenceTableContextMenu;
  delete filterForm;
  delete evidenceEditor;
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
}

void EvidenceManager::buildUi() {
  gridLayout = new QGridLayout(this);
  filterForm = new EvidenceFilterForm(this);
  evidenceTableContextMenu = new QMenu(this);
  submitEvidenceAction = new QAction("Submit Evidence", evidenceTableContextMenu);
  evidenceTableContextMenu->addAction(submitEvidenceAction);
  deleteEvidenceAction = new QAction("Delete Evidence", evidenceTableContextMenu);
  evidenceTableContextMenu->addAction(deleteEvidenceAction);
  copyPathToClipboardAction = new QAction("Copy Path", evidenceTableContextMenu);
  evidenceTableContextMenu->addAction(copyPathToClipboardAction);

  filterTextBox = new QLineEdit(this);
  editFiltersButton = new QPushButton("Edit Filters", this);
  applyFilterButton = new QPushButton("Apply", this);
  resetFilterButton = new QPushButton("Reset", this);

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
    3  | Loading Ani   | Hor Spacer  | <None>     | <None>      |
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

  closeWindowAction = new QAction(this);
  closeWindowAction->setShortcut(QKeySequence::Close);
  this->addAction(closeWindowAction);

  this->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);
  this->resize(800, 600);
  this->setWindowTitle("Evidence Manager");
  this->setLayout(gridLayout);
}

void EvidenceManager::wireUi() {
  // remap events to shorten the connect lines
  auto btnClicked = &QPushButton::clicked;
  auto actionTriggered = &QAction::triggered;

  connect(applyFilterButton, btnClicked, this, &EvidenceManager::loadEvidence);
  connect(resetFilterButton, btnClicked, this, &EvidenceManager::resetFilterButtonClicked);
  connect(editFiltersButton, btnClicked, this, &EvidenceManager::openFiltersMenu);

  connect(filterTextBox, &QLineEdit::returnPressed, this, &EvidenceManager::loadEvidence);

  connect(submitEvidenceAction, actionTriggered, this, &EvidenceManager::submitEvidenceTriggered);
  connect(deleteEvidenceAction, actionTriggered, this, &EvidenceManager::deleteEvidenceTriggered);
  connect(closeWindowAction, actionTriggered, this, &EvidenceManager::close);
  connect(copyPathToClipboardAction, actionTriggered, this, &EvidenceManager::copyPathTriggered);

  connect(filterForm, &EvidenceFilterForm::evidenceSet, this, &EvidenceManager::applyFilterForm);

  connect(this, &EvidenceManager::evidenceChanged, evidenceEditor, &EvidenceEditor::updateEvidence);
  connect(evidenceTable, &QTableWidget::currentCellChanged, this, &EvidenceManager::onRowChanged);
  connect(evidenceTable, &QTableWidget::customContextMenuRequested, this,
          &EvidenceManager::openTableContextMenu);
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
      QMessageBox::warning(this, "Cannot submit evidence",
                           "Could not retrieve data. Please try again.");
    }
  }
}

void EvidenceManager::deleteEvidenceTriggered() {
  auto reply = QMessageBox::question(this, "Discard Evidence",
                                     "Are you sure you want to discard this evidence? This will "
                                     "only delete this evidence on your computer.",
                                     QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

  if (reply == QMessageBox::Yes) {
    auto deleteResp = evidenceEditor->deleteEvidence();
    auto evi = deleteResp.model;

    if (!deleteResp.dbDeleteSuccess) {
      std::cout << "Could not delete evidence from internal database. Error: "
                << deleteResp.errorText.toStdString() << std::endl;
    }
    else if (!deleteResp.fileDeleteSuccess) {
      QMessageBox::warning(this, "Could not delete",
                           "Unable to delete evidence file.\n"
                           "You can try deleting the file directly. File Location:\n" +
                               evi.path);
    }
    else {
      loadEvidence();
    }
  }
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
  setColText(COL_SUBMITTED, model.uploadDate.isNull() ? "No" : "Yes");
  setColText(COL_FAILED, model.errorText == "" ? "" : "Yes");
  setColText(COL_PATH, model.path);
  setColText(COL_ERROR_MSG, model.errorText);

  auto uploadDateText = model.uploadDate.isNull() ? "Never" : model.uploadDate.toLocalTime().toString(dateFormat);
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

  QMessageBox::warning(this, "Cannot Save",
                       "Unable to save evidence data.\n"
                       "You can try uploading directly to the website. File Location:\n" +
                           saveResponse.model.path);
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

  if (currentRow == -1) {
    emit evidenceChanged(-1, true);
    return;
  }

  auto evidence = db->getEvidenceDetails(selectedRowEvidenceID());

  auto readonly = evidence.uploadDate.isValid();
  submitEvidenceAction->setEnabled(!readonly);
  emit evidenceChanged(evidence.id, true);
}

void EvidenceManager::onUploadComplete() {
  bool isValid;
  NetMan::extractResponse(uploadAssetReply, isValid);

  if (!isValid) {
    auto errMessage =
        "Unable to upload evidence: Network error (" + uploadAssetReply->errorString() + ")";
    try {
      db->updateEvidenceError(errMessage, evidenceIDForRequest);
    }
    catch (QSqlError& e) {
      std::cout << "Upload failed. Could not update internal database. Error: "
                << e.text().toStdString() << std::endl;
    }
    QMessageBox::warning(this, "Cannot Submit Evidence",
                         "Upload failed: Network error. Check your connection and try again.\n"
                         "(Error: " +
                             uploadAssetReply->errorString() + ")");
  }
  else {
    try {
      db->updateEvidenceSubmitted(evidenceIDForRequest);
    }
    catch (QSqlError& e) {
      std::cout << "Upload successful. Could not update internal database. Error: "
                << e.text().toStdString() << std::endl;
    }
    emit evidenceChanged(evidenceIDForRequest, true);  // lock the editing form
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
