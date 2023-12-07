#include "evidenceeditor.h"

#include <QFile>
#include <QTextEdit>
#include <QSplitter>
#include "components/evidencepreview.h"
#include "db/databaseconnection.h"
#include "components/aspectratio_pixmap_label/imageview.h"
#include "components/code_editor/codeblockview.h"
#include "components/error_view/errorview.h"
#include "components/evidence_editor/evidenceeditor.h"
#include "components/tagging/tageditor.h"
#include "models/codeblock.h"
#include "models/evidence.h"

EvidenceEditor::EvidenceEditor(qint64 evidenceID, DatabaseConnection *db, QWidget *parent)
    : EvidenceEditor(db, parent)
{
  this->evidenceID = evidenceID;
  loadData();
}

EvidenceEditor::EvidenceEditor(DatabaseConnection *db, QWidget *parent)
  : QWidget(parent)
  , db(db)
  , splitter(new QSplitter(this))
  , tagEditor(new TagEditor(this))
  , descriptionTextBox(new QTextEdit(this))
{
  buildUi();
  setEnabled(false);
}

void EvidenceEditor::buildUi() {

  connect(tagEditor, &TagEditor::tagsLoaded, this, &EvidenceEditor::onTagsLoaded);

  splitter->setOrientation(Qt::Vertical);
  splitter->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

  auto descriptionAreaLayout = new QVBoxLayout();
  descriptionAreaLayout->addWidget(new QLabel(tr("Description"), this));
  descriptionAreaLayout->addWidget(descriptionTextBox);
  descriptionAreaLayout->setContentsMargins(0, 0, 0, 0);

  auto descriptionArea = new QWidget(this);
  splitter->addWidget(descriptionArea);
  descriptionArea->setLayout(descriptionAreaLayout);

  /* mainLayout
       +----------------------------------------+
    0  | +------------Vert. Splitter---------+  |
       | |                                   |  |
       | |        Preview Area (reserved)    |  |
       | +-----------------------------------+  |
       | |  +-----------------------------+  |  |
       | |  | description label           |  |  |
       | |  +-----------------------------|  |  |
       | |  |       Desc Text box         |  |  |
       | |  |        (minimized)          |  |  |
       | |  +-----------------------------|  |  |
       | +-----------------------------------+  |
       +----------------------------------------+
    1  |                                        |
       |                 tagEditor              |
       |                                        |
       +----------------------------------------+
  */
  auto mainLayout = new QVBoxLayout(this);
  mainLayout->setContentsMargins(0, 0, 0, 0);
  mainLayout->addWidget(splitter);
  mainLayout->addWidget(tagEditor);
  setLayout(mainLayout);
}

model::Evidence EvidenceEditor::encodeEvidence() {
  model::Evidence copy = model::Evidence(originalEvidenceData);

  copy.description = descriptionTextBox->toPlainText();
  copy.tags.clear();
  copy.tags = tagEditor->getIncludedTags();

  return copy;
}

void EvidenceEditor::setEnabled(bool enable) {
  // if the product is enabled, then we can edit, hence it's not readonly
  descriptionTextBox->setReadOnly(!enable);
  tagEditor->setReadonly(!enable);
  if (loadedPreview != nullptr) {
    loadedPreview->setReadonly(!enable);
  }
}

void EvidenceEditor::loadData()
{
    // get local db evidence data
    clearEditor();
    originalEvidenceData = db->getEvidenceDetails(evidenceID);
    if(originalEvidenceData.id == -1) {
        loadedPreview = new ErrorView(tr("Unable to load evidence: %1").arg(db->errorString()), this);
        return;
    }

    descriptionTextBox->setText(originalEvidenceData.description);
    operationSlug = originalEvidenceData.operationSlug;
    if (originalEvidenceData.contentType == QStringLiteral("image")) {
        loadedPreview = new ImageView(this);
        loadedPreview->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    } else if (originalEvidenceData.contentType == QStringLiteral("codeblock")) {
        loadedPreview = new CodeBlockView(this);
        loadedPreview->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    } else {
        loadedPreview = new ErrorView(tr("Unsupported evidence type: %1").arg(originalEvidenceData.contentType), this);
    }
    loadedPreview->loadFromFile(originalEvidenceData.path);
    loadedPreview->setReadonly(readonly);
    // get all remote tags (for op)
    tagEditor->loadTags(operationSlug, originalEvidenceData.tags);
    splitter->insertWidget(0, loadedPreview);
}

void EvidenceEditor::revert() {
  tagEditor->clear();
  originalEvidenceData = db->getEvidenceDetails(evidenceID);
  descriptionTextBox->setText(originalEvidenceData.description);
  tagEditor->loadTags(operationSlug, originalEvidenceData.tags);
}

void EvidenceEditor::updateEvidence(qint64 evidenceID, bool readonly) {
  clearEditor();
  setEnabled(false);
  this->readonly = readonly;
  this->evidenceID = evidenceID;
  if (evidenceID > 0) {
    loadData();
  }
}

void EvidenceEditor::clearEditor() {
  tagEditor->clear();
  descriptionTextBox->clear();
  if (loadedPreview != nullptr) {
    loadedPreview->clearPreview();
    delete loadedPreview;
    loadedPreview = nullptr;
  }
}

void EvidenceEditor::onTagsLoaded(bool success) {
  tagEditor->setReadonly(!success || readonly);
  Q_EMIT onWidgetReady();
}

// saveEvidence is a helper method to save (to the database) the currently
// loaded evidence, using the editor changes.
SaveEvidenceResponse EvidenceEditor::saveEvidence()
{
    if (loadedPreview != nullptr) {
        loadedPreview->saveEvidence();
    }
    auto evi = encodeEvidence();
    auto resp = SaveEvidenceResponse(evi);
    db->updateEvidenceDescription(evi.description, evi.id);
    if(!db->errorString().isEmpty()) {
        resp.actionSucceeded = false;
        resp.errorText = db->errorString();
        return resp;
    }

    db->setEvidenceTags(evi.tags, evi.id);
    if(!db->errorString().isEmpty()) {
        resp.actionSucceeded = false;
        resp.errorText = db->errorString();
        return resp;
    }
    resp.actionSucceeded = true;
    return resp;
}

QList<DeleteEvidenceResponse> EvidenceEditor::deleteEvidence(QList<qint64> evidenceIDs)
{
    QList<DeleteEvidenceResponse> responses;
    for (qint64 id : evidenceIDs) {
        model::Evidence evi = db->getEvidenceDetails(id);
        DeleteEvidenceResponse resp(evi);
        resp.dbDeleteSuccess = db->deleteEvidence(evi.id);
        if(!resp.dbDeleteSuccess)
            resp.errorText = db->errorString();

        auto localFile = QFile(evi.path);
        if (!localFile.remove()) {
            resp.fileDeleteSuccess = false;
            resp.errorText.append(QStringLiteral("\n%1").arg(localFile.errorString()));
        } else {
            resp.fileDeleteSuccess = true;
        }
        resp.errorText = resp.errorText.trimmed();
        responses.append(resp);
    }
    return responses;
}
