// Copyright 2020, Verizon Media
// Licensed under the terms of GPLv3. See LICENSE file in project root for terms.

#include "getinfo.h"

#include <QKeySequence>
#include <QMessageBox>

#include "appsettings.h"
#include "components/evidence_editor/evidenceeditor.h"
#include "helpers/netman.h"
#include "helpers/stopreply.h"
#include "helpers/ui_helpers.h"
#include "ui_getinfo.h"

GetInfo::GetInfo(DatabaseConnection* db, qint64 evidenceID, QWidget* parent)
    : QDialog(parent), ui(new Ui::GetInfo) {
  ui->setupUi(this);
  this->db = db;
  this->evidenceID = evidenceID;
  this->setAttribute(Qt::WA_DeleteOnClose);

  evidenceEditor = new EvidenceEditor(evidenceID, db, this);
  evidenceEditor->setEnabled(true);
  loadingButton = new LoadingButton(ui->submitButton->text(), this);

  UiHelpers::replacePlaceholder(ui->evidenceEditorPlaceholder, evidenceEditor, ui->gridLayout);
  UiHelpers::replacePlaceholder(ui->submitButton, loadingButton, ui->gridLayout);

  closeWindowAction = new QAction(this);
  closeWindowAction->setShortcut(QKeySequence::Close);
  this->addAction(closeWindowAction);

  wireUi();

  // Make the dialog pop up above any other windows but retain title bar and buttons
  Qt::WindowFlags flags = this->windowFlags();
  flags |= Qt::CustomizeWindowHint | Qt::WindowStaysOnTopHint | Qt::WindowMinMaxButtonsHint |
           Qt::WindowCloseButtonHint;
  this->setWindowFlags(flags);
}

GetInfo::~GetInfo() {
  delete ui;
  delete evidenceEditor;
  delete loadingButton;
  delete closeWindowAction;
  stopReply(&uploadAssetReply);
}

void GetInfo::wireUi() {
  connect(loadingButton, &QPushButton::clicked, this, &GetInfo::submitButtonClicked);
  connect(ui->deleteButton, &QPushButton::clicked, this, &GetInfo::deleteButtonClicked);
  connect(closeWindowAction, &QAction::triggered, this, &GetInfo::deleteButtonClicked);
}

void GetInfo::showEvent(QShowEvent* evt) {
  QDialog::showEvent(evt);
  setFocus();  // giving the form focus, to prevent retaining focus on the submit button when
               // closing the window
}

bool GetInfo::saveData() {
  auto saveResponse = evidenceEditor->saveEvidence();
  if (!saveResponse.actionSucceeded) {
    QMessageBox::warning(this, "Cannot Save",
                         "Unable to save evidence data.\n"
                         "You can try uploading directly to the website. File Location:\n" +
                             saveResponse.model.path);
  }
  return saveResponse.actionSucceeded;
}

void GetInfo::submitButtonClicked() {
  loadingButton->startAnimation();
  setActionButtonsEnabled(false);
  if (saveData()) {
    try {
      model::Evidence evi = db->getEvidenceDetails(evidenceID);
      uploadAssetReply = NetMan::getInstance().uploadAsset(evi);
      connect(uploadAssetReply, &QNetworkReply::finished, this, &GetInfo::onUploadComplete);
    }
    catch (QSqlError& e) {
      QMessageBox::warning(this, "Cannot submit evidence",
                           "Could not retrieve data. Please try again.");
    }
  }
}

void GetInfo::deleteButtonClicked() {
  auto reply = QMessageBox::question(this, "Discard Evidence",
                                     "Are you sure you want to discard this evidence?",
                                     QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

  if (reply == QMessageBox::Yes) {
    setActionButtonsEnabled(false);
    bool shouldClose = true;

    model::Evidence evi = evidenceEditor->encodeEvidence();
    if (!QFile::remove(evi.path)) {
      QMessageBox::warning(this, "Could not delete",
                           "Unable to delete evidence file.\n"
                           "You can try deleting the file directly. File Location:\n" +
                               evi.path);
      shouldClose = false;
    }
    try {
      db->deleteEvidence(evidenceID);
    }
    catch (QSqlError& e) {
      std::cout << "Could not delete evidence from internal database. Error: "
                << e.text().toStdString() << std::endl;
    }

    setActionButtonsEnabled(true);
    if (shouldClose) {
      this->close();
    }
  }
}

void GetInfo::setActionButtonsEnabled(bool enabled) {
  loadingButton->setEnabled(enabled);
  ui->deleteButton->setEnabled(enabled);
}

void GetInfo::onUploadComplete() {
  if (uploadAssetReply->error() != QNetworkReply::NoError) {
    auto errMessage =
        "Unable to upload evidence: Network error (" + uploadAssetReply->errorString() + ")";
    try {
      db->updateEvidenceError(errMessage, this->evidenceID);
    }
    catch (QSqlError& e) {
      std::cout << "Upload failed. Could not update internal database. Error: "
                << e.text().toStdString() << std::endl;
    }
    QMessageBox::warning(this, "Cannot submit evidence",
                         "Upload failed: Network error. Check your connection and try again.\n"
                         "Note: This evidence has been saved. You can close this window and "
                         "re-submit from the evidence manager."
                         "\n(Error: " +
                             uploadAssetReply->errorString() + ")");
  }
  else {
    try {
      db->updateEvidenceSubmitted(this->evidenceID);
      emit evidenceSubmitted(db->getEvidenceDetails(this->evidenceID));
      this->close();
    }
    catch (QSqlError& e) {
      std::cout << "Upload successful. Could not update internal database. Error: "
                << e.text().toStdString() << std::endl;
    }
  }
  // we don't actually need anything from the uploadAssets reply, so just clean it up.
  // one thing we might want to record: evidence uuid... not sure why we'd need it though.
  loadingButton->stopAnimation();
  setActionButtonsEnabled(true);
  tidyReply(&uploadAssetReply);
}
