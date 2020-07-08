#ifndef TAGEDITOR_H
#define TAGEDITOR_H

#include <QWidget>
#include <QGridLayout>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include <QNetworkReply>
#include <QErrorMessage>

#include "components/tagging/tagwidget.h"
#include "components/tagging/tagview.h"
#include "components/loading_button/loadingbutton.h"
#include "components/loading/qprogressindicator.h"
#include "models/tag.h"

class TagEditor : public QWidget
{
  Q_OBJECT
 public:
  explicit TagEditor(QWidget *parent = nullptr);
  ~TagEditor();

 private:
  void buildUi();
  void wireUi();

  void addButtonClicked();
  void createTag(QString tagName);

 private slots:
  void onGetTagsComplete();
  void onCreateTagComplete();

 public:
  void clear();
  void setReadonly(bool readonly);
  void loadTags(const QString& operationSlug, std::vector<model::Tag> initialTagIDs);
  std::vector<model::Tag> getIncludedTags();

 signals:
  void tagsLoaded(bool isValid);

 private:
  QString operationSlug = "";
  std::vector<model::Tag> initialTags;

  QNetworkReply* getTagsReply = nullptr;
  QNetworkReply* createTagReply = nullptr;

  QErrorMessage* couldNotCreateTagMsg = nullptr;

  // Ui Elements
  QGridLayout* gridLayout = nullptr;
  QComboBox* tagListComboBox = nullptr;
  QProgressIndicator* loading = nullptr;
  QLabel* errorLabel = nullptr;
  TagView* tagView = nullptr;
};

#endif // TAGEDITOR_H
