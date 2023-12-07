#include "getinfo.h"

#include <QGridLayout>
#include <QMessageBox>

#include "components/evidence_editor/evidenceeditor.h"
#include "components/loading_button/loadingbutton.h"
#include "db/databaseconnection.h"
#include "helpers/netman.h"
#include "helpers/cleanupreply.h"

GetInfo::GetInfo(DatabaseConnection* db, qint64 evidenceID, QWidget* parent)
    : AShirtDialog(parent, AShirtDialog::commonWindowFlags)
    , db(db)
    , evidenceID(evidenceID)
    , submitButton(new LoadingButton(tr("Submit"), this))
    , evidenceEditor(new EvidenceEditor(this->evidenceID, this->db, this))
{
  buildUi();
  wireUi();
}

GetInfo::~GetInfo() {
  delete evidenceEditor;
  cleanUpReply(&uploadAssetReply);
}

void GetInfo::buildUi() {
  submitButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  submitButton->setAutoDefault(false);
  connect(submitButton, &QPushButton::clicked, this, &GetInfo::submitButtonClicked);
  connect(this, &GetInfo::setActionButtonsEnabled, submitButton, &QPushButton::setEnabled);

  auto deleteButton = new QPushButton(tr("Delete"), this);
  deleteButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  deleteButton->setAutoDefault(false);
  connect(deleteButton, &QPushButton::clicked, this, &GetInfo::deleteButtonClicked);
  connect(this, &GetInfo::setActionButtonsEnabled, deleteButton, &QPushButton::setEnabled);

  evidenceEditor->setEnabled(true);

  // Layout
  /*        0                 1            2
       +---------------+-------------+------------+
    0  |                                          |
       |             Evidence Editor              |
       |                                          |
       +---------------+-------------+------------+
    1  | Delete Btn    | <None>      | Submit Btn |
       +---------------+-------------+------------+
  */
  auto gridLayout = new QGridLayout(this);
  gridLayout->addWidget(evidenceEditor, 0, 0, 1, 3);
  gridLayout->addWidget(deleteButton, 1, 0);
  gridLayout->addWidget(submitButton, 1, 2);
  setLayout(gridLayout);

  setAttribute(Qt::WA_DeleteOnClose);
  resize(720, 480);
  setWindowTitle(tr("Add Evidence Details"));
  setFocus(); // ensure focus is not on the submit button
}

void GetInfo::wireUi() {
}

void GetInfo::showEvent(QShowEvent* evt) {
  QDialog::showEvent(evt);
  setFocus();  // giving the form focus, to prevent retaining focus on the submit button when
               // closing the window
}

bool GetInfo::saveData() {
  auto saveResponse = evidenceEditor->saveEvidence();
  if (!saveResponse.actionSucceeded) {
    QMessageBox::warning(this, tr("Cannot Save"),
                         tr("Unable to save evidence data.\n"
                         "You can try uploading directly to the website. File Location:\n%1")
                           .arg(saveResponse.model.path));
  }
  return saveResponse.actionSucceeded;
}

void GetInfo::submitButtonClicked()
{
    submitButton->startAnimation();
    Q_EMIT setActionButtonsEnabled(false);
    if (!saveData())
        return;
    model::Evidence evi = db->getEvidenceDetails(evidenceID);
    if(evi.id == -1) {
        QMessageBox::warning(this, tr("Cannot submit evidence"),
                             tr("Could not retrieve data. Please try again."));
        return;
    }
    uploadAssetReply = NetMan::uploadAsset(evi);
    connect(uploadAssetReply, &QNetworkReply::finished, this, &GetInfo::onUploadComplete);
}

void GetInfo::deleteButtonClicked() {
  auto reply = QMessageBox::question(this, tr("Discard Evidence"),
                                     tr("Are you sure you want to discard this evidence?"),
                                     QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

  if (reply == QMessageBox::Yes) {
    Q_EMIT  setActionButtonsEnabled(false);
    bool shouldClose = true;

    model::Evidence evi = evidenceEditor->encodeEvidence();
    if (!QFile::remove(evi.path)) {
      QMessageBox::warning(this, tr("Could not delete"),
                           tr("Unable to delete evidence file.\n"
                           "You can try deleting the file directly. File Location:\n%1")
                             .arg(evi.path));
      shouldClose = false;
    }

    db->deleteEvidence(evidenceID);

    Q_EMIT setActionButtonsEnabled(true);
    if (shouldClose) {
      close();
    }
  }
}

void GetInfo::onUploadComplete()
{
    if (uploadAssetReply->error() != QNetworkReply::NoError) {
        auto errMessage = tr("Unable to upload evidence: Network error (%1)").arg(uploadAssetReply->errorString());
        db->updateEvidenceError(errMessage, evidenceID);
        if(!db->errorString().isEmpty())
            qWarning() << "Upload failed. Could not update internal database. Error: " << db->errorString();
        QMessageBox::warning(this, tr("Cannot submit evidence"),
                             tr("Upload failed: Network error. Check your connection and try again.\n"
                                "Note: This evidence has been saved. You can close this window and "
                                "re-submit from the evidence manager."
                                "\n(Error: %1)").arg(uploadAssetReply->errorString()));
    } else {
        db->updateEvidenceSubmitted(evidenceID);
        if(!db->errorString().isEmpty())
            qWarning() << "Upload successful. Could not update internal database. Error: " << db->errorString();
        Q_EMIT evidenceSubmitted(db->getEvidenceDetails(evidenceID));
        close();
    }
    // we don't actually need anything from the uploadAssets reply, so just clean it up.
    // one thing we might want to record: evidence uuid... not sure why we'd need it though.
    submitButton->stopAnimation();
    Q_EMIT setActionButtonsEnabled(true);
    cleanUpReply(&uploadAssetReply);
}
