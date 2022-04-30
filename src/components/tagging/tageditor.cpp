#include "components/tagging/tageditor.h"

#include <QAbstractItemView>
#include <QLineEdit>
#include <QStringListModel>
#include <QTimer>
#include <algorithm>

#include "helpers/netman.h"
#include "helpers/stopreply.h"

TagEditor::TagEditor(QWidget *parent) : QWidget(parent) {
  buildUi();
  tagCache = new TagCache();
  wireUi();
}

TagEditor::~TagEditor() {
  delete couldNotCreateTagMsg;
  delete loading;
  delete tagCompleteTextBox;
  delete errorLabel;
  delete tagView;
  delete gridLayout;
  delete completer;

  delete tagCache;
  for (auto entry : activeRequests) {
    stopReply(&(entry.second));
  }
  stopReply(&createTagReply);
}

void TagEditor::buildUi() {
  gridLayout = new QGridLayout(this);
  gridLayout->setContentsMargins(0, 0, 0, 0);

  couldNotCreateTagMsg = new QErrorMessage(this);

  tagView = new TagView(this);
  errorLabel = new QLabel(this);
  loading = new QProgressIndicator(this);

  completer = new QCompleter(this);
  completer->setCompletionMode(QCompleter::PopupCompletion);
  completer->setFilterMode(Qt::MatchContains);
  completer->setCaseSensitivity(Qt::CaseInsensitive);

  tagCompleteTextBox = new QLineEdit(this);
  tagCompleteTextBox->setPlaceholderText("Add Tags...");
  tagCompleteTextBox->installEventFilter(&filter);
  tagCompleteTextBox->setCompleter(completer);
  tagCompleteTextBox->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed));

  // Layout
  /*        0                 1           2
       +----------------+-------------+--------------+
    0  |                                             |
       |                 Flow Tag List               |
       |                                             |
       +----------------+-------------+--------------+
    1  | errLbl/Loading | <None>      | [Add Tag TB] |
       +----------------+-------------+--------------+
  */

  // row 0
  gridLayout->addWidget(tagView, 0, 0, 1, 3);

  // row 1
  gridLayout->addWidget(errorLabel, 1, 0);
  gridLayout->addWidget(loading, 1, 0);
  gridLayout->addWidget(tagCompleteTextBox, 1, 2);

  this->setLayout(gridLayout);
}

void TagEditor::wireUi() {
  connect(tagCompleteTextBox, &QLineEdit::returnPressed, this, &TagEditor::tagEditReturnPressed);
  connect(&filter, &TaggingLineEditEventFilter::upPressed, this, &TagEditor::showCompleter);
  connect(&filter, &TaggingLineEditEventFilter::downPressed, this, &TagEditor::showCompleter);
  connect(&filter, &TaggingLineEditEventFilter::completePressed, this, &TagEditor::showCompleter);
  connect(&filter, &TaggingLineEditEventFilter::leftMouseClickPressed, this, &TagEditor::showCompleter);

  connect(completer, QOverload<const QString &>::of(&QCompleter::activated), this,
          &TagEditor::completerActivated);

  connect(tagCompleteTextBox, &QLineEdit::textChanged, [this](const QString &text) {
    if (text.isEmpty()) {
      tagCompleteTextBox->completer()->setCompletionPrefix(QString());
    }
  });

  connect(tagCache, &TagCache::tagResponse, this, &TagEditor::tagsUpdated);
  connect(tagCache, &TagCache::failedLookup, this, &TagEditor::tagsNotFound);
}

void TagEditor::completerActivated(const QString &text) {
  tagTextEntered(text);
  QTimer::singleShot(0, tagCompleteTextBox, &QLineEdit::clear);
}

void TagEditor::tagEditReturnPressed() {
  if (completer->popup()->isVisible()) {
    return;
  }
  tagTextEntered(tagCompleteTextBox->text().trimmed());
}

void TagEditor::tagTextEntered(QString text) {
  if (text.isEmpty()) {
    return;
  }

  auto foundTag = tagMap.find(standardizeTagKey(text));
  if (foundTag == tagMap.end()) {
    createTag(text);
  }
  else {
    dto::Tag data = foundTag->second;
    tagView->contains(data) ? tagView->remove(data) : tagView->addTag(data);
  }

  tagCompleteTextBox->clear();
  tagCompleteTextBox->completer()->setCompletionPrefix(QString());
}

void TagEditor::updateCompleterModel() {
  tagNames.sort(Qt::CaseInsensitive);
  // no need to delete previous model -- handled by qcompleter
  completer->setModel(new QStringListModel(tagNames, completer));
  completer->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
}

void TagEditor::clear() {
  stopReply(&createTagReply);
  tagCompleteTextBox->clear();
  errorLabel->clear();
  tagView->clear();
}

void TagEditor::loadTags(const QString &operationSlug, std::vector<model::Tag> initialTags) {
  this->operationSlug = operationSlug;
  this->initialTags = initialTags;

  tagCache->requestTags(operationSlug);
}

void TagEditor::tagsUpdated(QString operationSlug, std::vector<dto::Tag> tags) {
  if (this->operationSlug == operationSlug) {
    clearTags();
    for (auto tag : tags) {
      addTag(tag);

      auto itr = std::find_if(initialTags.begin(), initialTags.end(), [tag](model::Tag modelTag) {
        return modelTag.serverTagId == tag.id;
      });
      if (itr != initialTags.end()) {
        tagView->addTag(tag);
      }
    }
    updateCompleterModel();
    Q_EMIT tagsLoaded(true);
  }
}

void TagEditor::tagsNotFound(QString operationSlug, std::vector<dto::Tag> outdatedTags) {
  if (this->operationSlug == operationSlug) {
    errorLabel->setText(
        tr("Unable to fetch tags."
           " Please check your connection."
           " (Tags names and colors may be incorrect)"));
    tagCompleteTextBox->setEnabled(false);
    // todo: factor in outdated data?
    for (auto tag : initialTags) {
      tagView->addTag(dto::Tag::fromModelTag(tag, TagWidget::randomColor()));
    }
    Q_EMIT tagsLoaded(false);
  }
}

void TagEditor::createTag(QString tagName) {
  auto newText = tagName.trimmed();
  if (newText.isEmpty()) {
    return;
  }
  errorLabel->clear();
  loading->startAnimation();
  tagCompleteTextBox->setEnabled(false);

  dto::Tag newTag(newText, TagWidget::randomColor());
  createTagReply = NetMan::getInstance().createTag(newTag, operationSlug);
  connect(createTagReply, &QNetworkReply::finished, this, &TagEditor::onCreateTagComplete);
}

void TagEditor::onCreateTagComplete() {
  bool isValid;
  auto data = NetMan::extractResponse(createTagReply, isValid);
  if (isValid) {
    auto newTag = dto::Tag::parseData(data);
    addTag(newTag);
    tagView->addTag(newTag);
    tagCache->requestExpiry(this->operationSlug);
    updateCompleterModel();
  }
  else {
    couldNotCreateTagMsg->showMessage(
        "Could not create tag."
        " Please check your connection and try again.");
  }
  disconnect(createTagReply, &QNetworkReply::finished, this, &TagEditor::onCreateTagComplete);
  tidyReply(&createTagReply);
  loading->stopAnimation();
  tagCompleteTextBox->setEnabled(true);
  tagCompleteTextBox->setFocus();
}

void TagEditor::addTag(dto::Tag tag) {
  tagNames << tag.name;
  tagMap.emplace(standardizeTagKey(tag.name), tag);
}

void TagEditor::clearTags() {
  tagNames.clear();
  tagMap.clear();
}

QString TagEditor::standardizeTagKey(const QString& tagName) {
  return tagName.trimmed().toLower();
}

void TagEditor::setReadonly(bool readonly) {
  tagCompleteTextBox->setEnabled(!readonly);
  tagCompleteTextBox->setVisible(!readonly);

  tagView->setReadonly(readonly);
}
