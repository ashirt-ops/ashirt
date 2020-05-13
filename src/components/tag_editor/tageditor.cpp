#include "tageditor.h"

#include <QRandomGenerator>

#include "dtos/tag.h"
#include "helpers/netman.h"
#include "helpers/stopreply.h"

TagEditor::TagEditor(QWidget *parent) : QWidget(parent) {
  buildUi();
  couldNotCreateTagMsg = new QErrorMessage(this);

  wireUi();
}

TagEditor::~TagEditor() {
  delete _withTagsLabel;
  delete createTagTextBox;
  delete includedTagsListBox;
  delete allTagsListBox;
  delete includeTagButton;
  delete excludeTagButton;
  delete couldNotCreateTagMsg;
  delete createTagButton;
  delete gridLayout;

  stopReply(&getTagsReply);
  stopReply(&createTagReply);
}

void TagEditor::buildUi() {
  gridLayout = new QGridLayout(this);
  gridLayout->setMargin(0);

  _withTagsLabel = new QLabel("With Tags...", this);
  includedTagsListBox = new QListWidget(this);
  allTagsListBox = new QListWidget(this);
  includeTagButton = new QPushButton("<<", this);
  excludeTagButton = new QPushButton(">>", this);
  createTagTextBox = new QLineEdit();
  createTagButton = new LoadingButton("Create Tag", this);
  minorErrorLabel = new QLabel(this);

  // Layout
  /*        0          1           2          3
       +----------+-----------+----------+---------+
    0  | With Tags label                           |
       +----------+-----------+----------+---------+
    1  | Include  |  << btn   |     All            |
       +          +-----------+          +         +
    2  | Tag List |  >> Btn   |  Tags List         |
       +----------+-----------+----------+---------+
    3  | err Lab  | <empty>   | Add TB   | add btn |
       +----------+-----------+----------+---------+
  */

  // row 0
  gridLayout->addWidget(_withTagsLabel, 0, 0, 1, gridLayout->columnCount());

  // row 1
  gridLayout->addWidget(includedTagsListBox, 1, 0, 2, 1);
  gridLayout->addWidget(includeTagButton, 1, 1);
  gridLayout->addWidget(allTagsListBox, 1, 2, 2, 2);

  // row 2
  gridLayout->addWidget(excludeTagButton, 2, 1);

  // row 3
  gridLayout->addWidget(minorErrorLabel, 3, 0);
  gridLayout->addWidget(createTagTextBox, 3, 2);
  gridLayout->addWidget(createTagButton, 3, 3);

  this->setLayout(gridLayout);
}

void TagEditor::wireUi() {
  auto btnClicked = &QPushButton::clicked;
  connect(createTagButton, btnClicked, this, &TagEditor::createTagButtonClicked);
  connect(includeTagButton, btnClicked, this, &TagEditor::includeTagButtonClicked);
  connect(excludeTagButton, btnClicked, this, &TagEditor::excludeTagButtonClicked);
  connect(createTagTextBox, &QLineEdit::returnPressed, this, &TagEditor::createTagButtonClicked);

  allTagsListBox->setSortingEnabled(true);
}

void TagEditor::clear() {
  stopReply(&getTagsReply);
  stopReply(&createTagReply);
  createTagTextBox->setText("");
  allTagsListBox->clear();
  includedTagsListBox->clear();
  minorErrorLabel->setText("");
}

void TagEditor::setEnabled(bool enable) {
  createTagButton->setEnabled(enable);
  includeTagButton->setEnabled(enable);
  excludeTagButton->setEnabled(enable);
  createTagTextBox->setEnabled(enable);
}

void TagEditor::loadTags(const QString& operationSlug, std::vector<qint64> initialTagIDs) {
  this->operationSlug = operationSlug;

  includedTagIds.clear();
  for (auto tagID : initialTagIDs) {
    includedTagIds.insert(tagID);
  }

  getTagsReply = NetMan::getInstance().getOperationTags(operationSlug);
  connect(getTagsReply, &QNetworkReply::finished, this, &TagEditor::onGetTagsComplete);
}

void TagEditor::refreshTagBoxes() {
  allTagsListBox->clear();
  includedTagsListBox->clear();

  for (const auto &itr : knownTags) {
    QString tagText = itr.first;
    qint64 tagId = itr.second;
    int itemCount = includedTagIds.count(tagId);

    if (itemCount == 1) {
      includedTagsListBox->addItem(tagText);
    }
    else {
      allTagsListBox->addItem(tagText);
    }
  }
}

std::vector<model::Tag> TagEditor::getIncludedTags() {
  std::vector<model::Tag> rtn;
  rtn.reserve(includedTagIds.size());

  // Construct a reverse map to find tag names.
  // slightly inefficient way to do this, but much easier to code against.
  std::unordered_map<qint64, QString> reversedMap;
  for (const auto &entry : knownTags) {
    reversedMap.insert(std::pair<qint64, QString>(entry.second, entry.first));
  }

  for (const qint64 &tagID : includedTagIds) {
    try {
      auto tagName = reversedMap.at(tagID);
      model::Tag tag(tagID, tagName);
      rtn.push_back(tag);
    }
    catch (std::out_of_range &e) {
    }  // drop any tag ids we can't find (doesn't exist on the server, and will fail anyway)
  }

  return rtn;
}

void TagEditor::createTagButtonClicked() {
  auto newText = createTagTextBox->text().trimmed();
  if (newText == "") {
    return;
  }

  createTagButton->startAnimation();
  createTagButton->setEnabled(false);

  dto::Tag newTag(newText, randomColor());

  createTagReply = NetMan::getInstance().createTag(newTag, operationSlug);
  connect(createTagReply, &QNetworkReply::finished, this, &TagEditor::onCreateTagComplete);
}

void TagEditor::includeTagButtonClicked() {
  auto items = allTagsListBox->selectedItems();
  for (auto item : items) {
    includedTagsListBox->addItem(item->text());
    allTagsListBox->takeItem(allTagsListBox->row(item));
    includedTagIds.insert(knownTags[item->text()]);
  }
}

void TagEditor::excludeTagButtonClicked() {
  auto items = includedTagsListBox->selectedItems();
  for (auto item : items) {
    allTagsListBox->addItem(item->text());
    includedTagsListBox->takeItem(includedTagsListBox->row(item));
    includedTagIds.erase(knownTags[item->text()]);
  }
}

void TagEditor::onGetTagsComplete() {
  bool isValid;
  auto data = NetMan::extractResponse(getTagsReply, isValid);
  if (isValid) {
    std::vector<dto::Tag> tags = dto::Tag::parseDataAsList(data);
    knownTags.clear();

    for (const dto::Tag &tag : tags) {
      knownTags.insert(std::pair<QString, qint64>(tag.name, tag.id));
    }
    refreshTagBoxes();
  }
  else {
    minorErrorLabel->setText(tr("Unable to fetch tags. Please check your connection."));
    includeTagButton->setEnabled(false);
  }

  disconnect(getTagsReply, &QNetworkReply::finished, this, &TagEditor::onGetTagsComplete);
  tidyReply(&getTagsReply);
  emit tagsLoaded(isValid);
}

void TagEditor::onCreateTagComplete() {
  bool isValid;
  auto data = NetMan::extractResponse(createTagReply, isValid);
  if (isValid) {
    auto newTag = dto::Tag::parseData(data);
    knownTags.insert(std::pair<QString, qint64>(newTag.name, newTag.id));
    includedTagIds.insert(newTag.id);
    createTagTextBox->setText("");
    refreshTagBoxes();
  }
  else {
    couldNotCreateTagMsg->showMessage(
        "Could not create tag. Please check your connection and try again.");
  }
  disconnect(createTagReply, &QNetworkReply::finished, this, &TagEditor::onCreateTagComplete);
  tidyReply(&createTagReply);
  createTagButton->stopAnimation();
  createTagButton->setEnabled(true);
}

QString TagEditor::randomColor() {
  // Note: this should match the frontend's color palette (naming)
  static std::vector<QString> colors = {
      "blue",      "yellow",      "green",      "indigo",         "orange",
      "lightBlue", "lightYellow", "lightGreen", "lightIndigo",    "lightOrange",
      "pink",      "red",         "teal",       "vermilion",      "violet",
      "lightPink", "lightRed",    "lightTeal",  "lightVermilion", "lightViolet"};
  auto index = QRandomGenerator::global()->bounded(int(colors.size()));
  return colors.at(index);
}
