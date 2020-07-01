// Copyright 2020, Verizon Media
// Licensed under the terms of GPLv3. See LICENSE file in project root for terms.

#include "evidencemanager.h"

#include <QCheckBox>
#include <QMessageBox>
#include <QRandomGenerator>
#include <QStandardPaths>
#include <QTableWidgetItem>
#include <iostream>

#include "appsettings.h"
#include "dtos/tag.h"
#include "forms/evidence_filter/evidencefilter.h"
#include "forms/evidence_filter/evidencefilterform.h"
#include "helpers/netman.h"
#include "helpers/stopreply.h"
#include "helpers/ui_helpers.h"
#include "ui_evidencemanager.h"

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

EvidenceManager::EvidenceManager(DatabaseConnection* db, QWidget* parent)
    : QDialog(parent), ui(new Ui::EvidenceManager) {
  ui->setupUi(this);

  this->db = db;
  this->evidenceIDForRequest = 0;  // initializing to remove clang-tidy warning
  evidenceEditor = new EvidenceEditor(db, this);
  filterForm = new EvidenceFilterForm(this);
  submitButton =
      new LoadingButton(ui->submitEvidenceButton->text(), this, ui->submitEvidenceButton);

  // Replace the _evidenceEditorPlaceholder with a proper editor
  UiHelpers::replacePlaceholder(ui->_evidenceEditorPlaceholder, evidenceEditor, ui->gridLayout);
  evidenceEditor->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));

  UiHelpers::replacePlaceholder(ui->submitEvidenceButton, submitButton, ui->gridLayout);
  ui->submitEvidenceButton->setVisible(false);
  ui->gridLayout->removeWidget(ui->submitEvidenceButton);

  wireUi();
}

EvidenceManager::~EvidenceManager() {
  delete ui;
  delete evidenceEditor;
  delete filterForm;
  delete submitButton;
  stopReply(&uploadAssetReply);
}

void EvidenceManager::closeEvent(QCloseEvent* event) {
  QDialog::closeEvent(event);
  evidenceEditor->updateEvidence(-1, true);
}

void EvidenceManager::showEvent(QShowEvent* evt) {
  QDialog::showEvent(evt);
  resetFilterButtonClicked();
}

void EvidenceManager::wireUi() {
  auto btnClicked = &QPushButton::clicked;
  connect(submitButton, btnClicked, this, &EvidenceManager::submitEvidenceButtonClicked);
  connect(ui->deleteEvidenceButton, btnClicked, this,
          &EvidenceManager::deleteEvidenceButtonClicked);
  connect(ui->applyFilterButton, btnClicked, this, &EvidenceManager::applyFilterButtonClicked);
  connect(ui->resetFilterButton, btnClicked, this, &EvidenceManager::resetFilterButtonClicked);
  connect(ui->editFiltersButton, btnClicked, this, &EvidenceManager::openFiltersMenu);
  connect(ui->filterTextBox, &QLineEdit::returnPressed, this,
          &EvidenceManager::applyFilterButtonClicked);

  connect(filterForm, &EvidenceFilterForm::evidenceSet, this, &EvidenceManager::applyFilterForm);
  connect(ui->evidenceTable, &QTableWidget::currentCellChanged, this,
          &EvidenceManager::onRowChanged);
  connect(this, &EvidenceManager::evidenceChanged, evidenceEditor, &EvidenceEditor::updateEvidence);
}

void EvidenceManager::submitEvidenceButtonClicked() {
  submitButton->startAnimation();
  setActionButtonsEnabled(false);
  if (saveData()) {
    evidenceIDForRequest = selectedRowEvidenceID();
    try {
      model::Evidence evi = db->getEvidenceDetails(evidenceIDForRequest);
      uploadAssetReply = NetMan::getInstance().uploadAsset(evi);
      connect(uploadAssetReply, &QNetworkReply::finished, this, &EvidenceManager::onUploadComplete);
    }
    catch (QSqlError& e) {
      QMessageBox::warning(this, "Cannot submit evidence",
                           "Could not retrieve data. Please try again.");
    }
  }
}

void EvidenceManager::deleteEvidenceButtonClicked() {
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

void EvidenceManager::applyFilterButtonClicked() { loadEvidence(); }

void EvidenceManager::resetFilterButtonClicked() {
  EvidenceFilters filter;
  filter.operationSlug = AppSettings::getInstance().operationSlug();
  ui->filterTextBox->setText(filter.toString());
  loadEvidence();
}

void EvidenceManager::applyFilterForm(const EvidenceFilters& filter) {
  ui->filterTextBox->setText(filter.toString());
  applyFilterButtonClicked();
}

void EvidenceManager::loadEvidence() {
  enableEvidenceButtons(false);
  ui->evidenceTable->clearContents();

  try {
    auto filter = EvidenceFilters::parseFilter(ui->filterTextBox->text());
    std::vector<model::Evidence> operationEvidence = db->getEvidenceWithFilters(filter);
    ui->evidenceTable->setRowCount(operationEvidence.size());

    // removing sorting temporarily to solve a bug (per qt: not a bug)
    // Essentially, _not_ doing this breaks reloading the table. Mostly empty cells appear.
    // from: https://stackoverflow.com/a/8904287/4262552
    // see also: https://bugreports.qt.io/browse/QTBUG-75479
    ui->evidenceTable->setSortingEnabled(false);
    for (size_t row = 0; row < operationEvidence.size(); row++) {
      auto evi = operationEvidence.at(row);
      auto rowData = buildBaseEvidenceRow(evi.id);

      ui->evidenceTable->setItem(row, COL_OPERATION, rowData.operation);
      ui->evidenceTable->setItem(row, COL_DESCRIPTION, rowData.description);
      ui->evidenceTable->setItem(row, COL_CONTENT_TYPE, rowData.contentType);
      ui->evidenceTable->setItem(row, COL_DATE_CAPTURED, rowData.dateCaptured);
      ui->evidenceTable->setItem(row, COL_PATH, rowData.path);
      ui->evidenceTable->setItem(row, COL_FAILED, rowData.failed);
      ui->evidenceTable->setItem(row, COL_ERROR_MSG, rowData.errorText);
      ui->evidenceTable->setItem(row, COL_SUBMITTED, rowData.submitted);
      ui->evidenceTable->setItem(row, COL_DATE_SUBMITTED, rowData.dateSubmitted);

      setRowText(row, evi);
    }
    ui->evidenceTable->setSortingEnabled(true);
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
  static QString dateFormat = "MMM dd, yyyy hh:mm";

  ui->evidenceTable->item(row, COL_DATE_CAPTURED)->setText(model.recordedDate.toString(dateFormat));
  ui->evidenceTable->item(row, COL_DESCRIPTION)->setText(model.description);
  ui->evidenceTable->item(row, COL_OPERATION)->setText(model.operationSlug);
  ui->evidenceTable->item(row, COL_CONTENT_TYPE)->setText(model.contentType);
  ui->evidenceTable->item(row, COL_SUBMITTED)->setText(model.uploadDate.isNull() ? "No" : "Yes");
  ui->evidenceTable->item(row, COL_FAILED)->setText((model.errorText == "") ? "" : "Yes");
  ui->evidenceTable->item(row, COL_PATH)->setText(model.path);
  ui->evidenceTable->item(row, COL_ERROR_MSG)->setText(model.errorText);

  auto uploadDateText = model.uploadDate.isNull() ? "Never" : model.uploadDate.toString(dateFormat);
  ui->evidenceTable->item(row, COL_DATE_SUBMITTED)->setText(uploadDateText);
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

void EvidenceManager::setActionButtonsEnabled(bool enabled) {
  enableEvidenceButtons(enabled);
}

void EvidenceManager::enableEvidenceButtons(bool enabled) {
  submitButton->setEnabled(enabled);
  ui->deleteEvidenceButton->setEnabled(enabled);
}

bool EvidenceManager::saveData() {
  auto saveResponse = evidenceEditor->saveEvidence();
  if (saveResponse.actionSucceeded) {
    refreshRow(ui->evidenceTable->currentRow());
    return true;
  }

  QMessageBox::warning(this, "Cannot Save",
                       "Unable to save evidence data.\n"
                       "You can try uploading directly to the website. File Location:\n" +
                           saveResponse.model.path);
  return false;
}

void EvidenceManager::openFiltersMenu() {
  filterForm->setForm(EvidenceFilters::parseFilter(ui->filterTextBox->text()));
  filterForm->open();
}

void EvidenceManager::onRowChanged(int currentRow, int _currentColumn, int _previousRow,
                                   int _previousColumn) {
  Q_UNUSED(_currentColumn);
  Q_UNUSED(_previousRow);
  Q_UNUSED(_previousColumn);

  if (currentRow == -1) {
    enableEvidenceButtons(false);
    emit evidenceChanged(-1, true);
    return;
  }

  auto evidence = db->getEvidenceDetails(selectedRowEvidenceID());
  enableEvidenceButtons(true);

  auto readonly = evidence.uploadDate.isValid();
  submitButton->setEnabled(!readonly);
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
  refreshRow(ui->evidenceTable->currentRow());

  // we don't actually need anything from the uploadAssets reply, so just clean it up.
  // one thing we might want to record: evidence uuid... not sure why we'd need it though.
  submitButton->stopAnimation();

  tidyReply(&uploadAssetReply);
}

qint64 EvidenceManager::selectedRowEvidenceID() {
  return ui->evidenceTable->currentItem()->data(Qt::UserRole).toLongLong();
}
