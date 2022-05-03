#include "tagview.h"

#include <iostream>

TagView::TagView(QWidget *parent)
  : QWidget(parent)
{
  buildUi();
}

TagView::~TagView() {
  clear();
}

void TagView::buildUi() {

  auto tagGroupBox = new QGroupBox("Tags", this);
  layout = new FlowLayout();
  tagGroupBox->setLayout(layout);

  auto mainLayout = new QHBoxLayout(this);
  mainLayout->setContentsMargins(0, 0, 0, 0);
  mainLayout->addWidget(tagGroupBox);
}

void TagView::addTag(dto::Tag tag) {
  TagWidget* widget = new TagWidget(tag, readonly, this);
  includedTags.push_back(widget);
  layout->addWidget(widget);
  connect(widget, &TagWidget::removePressed, this, [this, widget](){
    removeWidget(widget);
  });
}

bool TagView::contains(dto::Tag tag) {
  for (const auto &widget : includedTags) {
    if (widget->getTag().id == tag.id) {
      return true;
    }
  }
  return false;
}

void TagView::removeWidget(TagWidget* tagWidget) {
  // remove from view
  tagWidget->hide();
  layout->removeWidget(tagWidget);

  // remove from includedTags
  auto itr = std::find(includedTags.begin(), includedTags.end(), tagWidget);
  if(itr != includedTags.cend()) {
    auto last = includedTags.end()-1;
    std::iter_swap(itr, last);
    includedTags.pop_back();
  }
  tagWidget->deleteLater();
}

void TagView::remove(dto::Tag tag) {
  auto itr = std::find_if(includedTags.begin(), includedTags.end(), [tag](TagWidget* item){
    return item->getTag().id == tag.id;
  });
  removeWidget(includedTags.at(itr - includedTags.begin()));
}

void TagView::clear() {
  for(auto widget : includedTags) {
    layout->removeWidget(widget);
    widget->deleteLater();
  }
  includedTags.clear();
}

std::vector<model::Tag> TagView::getIncludedTags() {
  std::vector<model::Tag> rtn;

  for (const auto &widget : includedTags) {
    dto::Tag tag = widget->getTag();
    rtn.push_back(model::Tag(tag.id, tag.name));
  }

  return rtn;
}

void TagView::setReadonly(bool readonly) {
  for(auto widget : includedTags) {
    widget->setReadOnly(readonly);
  }
}
