#include "components/tagging/tageditor.h"

#include <algorithm>
#include <QLineEdit>

#include "helpers/stopreply.h"
#include "helpers/netman.h"

TagEditor::TagEditor(QWidget *parent) : QWidget(parent) {
  buildUi();

  wireUi();
}

TagEditor::~TagEditor() {
  delete couldNotCreateTagMsg;
  delete loading;
  delete tagListComboBox;
  delete errorLabel;
  delete tagView;
  delete gridLayout;

  stopReply(&getTagsReply);
  stopReply(&createTagReply);
}

void TagEditor::buildUi() {
  gridLayout = new QGridLayout(this);
  gridLayout->setMargin(0);

  couldNotCreateTagMsg = new QErrorMessage(this);

  tagView = new TagView(this);
  errorLabel = new QLabel(this);
  loading = new QProgressIndicator(this);
  tagListComboBox = new QComboBox(this);
  tagListComboBox->setEditable(true);
  tagListComboBox->setInsertPolicy(QComboBox::NoInsert); // inserts will be handled by hand
  tagListComboBox->lineEdit()->setPlaceholderText("Add Tags...");

  // Layout
  /*        0                 1           2
       +----------------+-------------+--------------+
    0  |                                             |
       |                 Flow Tag List               |
       |                                             |
       +----------------+-------------+--------------+
    1  | errLbl/Loading | <None>      | [Add Tag CB] |
       +----------------+-------------+--------------+
  */

  // row 0
  gridLayout->addWidget(tagView, 0, 0, 1, 3);

  // row 1
  gridLayout->addWidget(errorLabel, 1, 0);
  gridLayout->addWidget(loading, 1, 0);
  gridLayout->addWidget(tagListComboBox, 1, 2);


  this->setLayout(gridLayout);
}

void TagEditor::wireUi() {
  connect(tagListComboBox->lineEdit(), &QLineEdit::returnPressed, this, &TagEditor::addButtonClicked);
  connect(tagListComboBox, &QComboBox::textActivated, this, &TagEditor::addButtonClicked);
}

void TagEditor::addButtonClicked() {
  auto text = tagListComboBox->currentText().trimmed();
  if (text.isEmpty()) {
    return;
  }

  int tagIndex = tagListComboBox->findText(text, Qt::MatchFixedString);
  if (tagIndex == -1) {
    createTag(text);
  }
  else {
    auto data = qvariant_cast<dto::Tag>(tagListComboBox->itemData(tagIndex));
    if (tagView->contains(data)) {
      tagView->remove(data);
    }
    else {
      tagView->addTag(data);
    }
  }

  tagListComboBox->setEditText("");
  tagListComboBox->setCurrentIndex(-1);
}

void TagEditor::clear() {
  stopReply(&getTagsReply);
  stopReply(&createTagReply);
  tagListComboBox->clear();
  errorLabel->setText("");
  tagView->clear();
}

void TagEditor::loadTags(const QString& operationSlug, std::vector<model::Tag> initialTags) {
  this->operationSlug = operationSlug;
  this->initialTags = initialTags;

  getTagsReply = NetMan::getInstance().getOperationTags(operationSlug);
  connect(getTagsReply, &QNetworkReply::finished, this, &TagEditor::onGetTagsComplete);
}

void TagEditor::onGetTagsComplete() {
  bool isValid;
  auto data = NetMan::extractResponse(getTagsReply, isValid);
  if (isValid) {
    std::vector<dto::Tag> tags = dto::Tag::parseDataAsList(data);
    for (auto tag : tags) {
      tagListComboBox->addItem(tag.name, QVariant::fromValue(tag));
      // If this exists in our initial tags, add it
      //auto itr = std::find(initialTags.begin(), initialTags.end(), tag.id); // TODO: this won't work
      auto itr = std::find_if(initialTags.begin(), initialTags.end(), [tag](model::Tag modelTag){
        return modelTag.serverTagId == tag.id;
      });
      if (itr != initialTags.end()) {
        tagView->addTag(tag);
      }
    }
    tagListComboBox->setCurrentIndex(-1);
  }
  else {
    errorLabel->setText(tr("Unable to fetch tags."
                           " Please check your connection."
                           " (Tags names and colors may be incorrect)"));
    tagListComboBox->setEnabled(false);
    for (auto tag : initialTags) {
      tagView->addTag(dto::Tag::fromModelTag(tag, TagWidget::randomColor()));
    }
  }

  disconnect(getTagsReply, &QNetworkReply::finished, this, &TagEditor::onGetTagsComplete);
  tidyReply(&getTagsReply);
  emit tagsLoaded(isValid);
}

void TagEditor::createTag(QString tagName) {
  auto newText = tagName.trimmed();
  if (newText == "") {
    return;
  }
  errorLabel->setText("");
  loading->startAnimation();
  tagListComboBox->setEnabled(false);

  dto::Tag newTag(newText, TagWidget::randomColor());
  createTagReply = NetMan::getInstance().createTag(newTag, operationSlug);
  connect(createTagReply, &QNetworkReply::finished, this, &TagEditor::onCreateTagComplete);
}

void TagEditor::onCreateTagComplete() {
  bool isValid;
  auto data = NetMan::extractResponse(createTagReply, isValid);
  if (isValid) {
    auto newTag = dto::Tag::parseData(data);
    tagListComboBox->addItem(newTag.name, QVariant::fromValue(newTag));
    tagView->addTag(newTag);
  }
  else {
    couldNotCreateTagMsg->showMessage("Could not create tag."
                                      " Please check your connection and try again.");
  }
  disconnect(createTagReply, &QNetworkReply::finished, this, &TagEditor::onCreateTagComplete);
  tidyReply(&createTagReply);
  loading->stopAnimation();
  tagListComboBox->setEnabled(true);
  tagListComboBox->setFocus();
}

std::vector<model::Tag> TagEditor::getIncludedTags() {
  return tagView->getIncludedTags();
}

void TagEditor::setReadonly(bool readonly) {
  tagListComboBox->setEnabled(!readonly);
  tagListComboBox->setVisible(!readonly);

  tagView->setReadonly(readonly);
}
