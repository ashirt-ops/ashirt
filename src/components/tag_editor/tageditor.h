#ifndef TAGEDITOR_H
#define TAGEDITOR_H

#include <QErrorMessage>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QNetworkReply>
#include <QPushButton>
#include <QWidget>

#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "components/loading_button/loadingbutton.h"
#include "models/tag.h"

class TagEditor : public QWidget {
  Q_OBJECT
 public:
  explicit TagEditor(QWidget* parent = nullptr);
  ~TagEditor();

 public:
  void setEnabled(bool enable);
  void loadTags(const QString& operationSlug, std::vector<qint64> initialTagIDs);
  void clear();
  std::vector<model::Tag> getIncludedTags();

 signals:
  void tagsLoaded(bool success);

 private:
  void buildUi();
  void wireUi();
  void refreshTagBoxes();
  QString randomColor();

 private slots:
  void createTagButtonClicked();
  void includeTagButtonClicked();
  void excludeTagButtonClicked();

  void onGetTagsComplete();
  void onCreateTagComplete();

 private:
  QString operationSlug;
  std::unordered_map<QString, qint64> knownTags;
  std::unordered_set<qint64> includedTagIds;

  // ui Components
  QGridLayout* gridLayout;
  QLabel* _withTagsLabel;
  LoadingButton* createTagButton;
  QLineEdit* createTagTextBox;
  QListWidget* includedTagsListBox;
  QListWidget* allTagsListBox;
  QPushButton* includeTagButton;
  QPushButton* excludeTagButton;
  QLabel* minorErrorLabel;

  QNetworkReply* getTagsReply = nullptr;
  QNetworkReply* createTagReply = nullptr;
  QErrorMessage* couldNotCreateTagMsg = nullptr;
};

#endif  // TAGEDITOR_H
