#pragma once

#include <QObject>
#include <QWidget>
#include <QGroupBox>

#include "components/tagging/tagwidget.h"
#include "components/flow_layout/flowlayout.h"
#include "models/tag.h"

class TagView : public QWidget
{
  Q_OBJECT
 public:
  explicit TagView(QWidget *parent = nullptr);
  ~TagView();

 private:
  void buildUi();
  void wireUi();

 private slots:
  void removeWidget(TagWidget* tag);

 public:
  void addTag(dto::Tag tag);
  std::vector<model::Tag> getIncludedTags();
  void setReadonly(bool readonly);
  bool contains(dto::Tag tag);
  void remove(dto::Tag tag);
  void clear();

 private:
  bool readonly = false;

  // UI Components
  QHBoxLayout* mainLayout = nullptr;
  FlowLayout* layout = nullptr;
  QGroupBox* tagGroupBox = nullptr;
  std::vector<TagWidget*> includedTags;
};
