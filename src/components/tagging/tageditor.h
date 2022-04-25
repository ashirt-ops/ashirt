#ifndef TAGEDITOR_H
#define TAGEDITOR_H

#include <QCompleter>
#include <QErrorMessage>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QNetworkReply>
#include <QPushButton>
#include <QWidget>

#include "components/loading/qprogressindicator.h"
#include "components/loading_button/loadingbutton.h"
#include "components/tagging/tagview.h"
#include "components/tagging/tagwidget.h"
#include "components/tagging/tagginglineediteventfilter.h"
#include "models/tag.h"

#include "tag_cache/tagcache.h"

class TagEditor : public QWidget {
  Q_OBJECT
 public:
  explicit TagEditor(QWidget* parent = nullptr);
  ~TagEditor();

 private:
  void buildUi();
  void wireUi();

  void createTag(QString tagName);
  void updateCompleterModel();
  void tagTextEntered(QString text);
  inline void showCompleter() { completer->complete(); }
  void addTag(dto::Tag tag);
  void clearTags();
  QString standardizeTagKey(const QString &tagName);

 private slots:
  void onCreateTagComplete();
  void tagEditReturnPressed();
  void completerActivated(const QString& text);

  void tagsUpdated(QString operationSlug, std::vector<dto::Tag> tags);
  void tagsNotFound(QString operationSlug, std::vector<dto::Tag> outdatedTags);

 public:
  void clear();
  void setReadonly(bool readonly);
  void loadTags(const QString& operationSlug, std::vector<model::Tag> initialTagIDs);
  inline std::vector<model::Tag> getIncludedTags() { return tagView->getIncludedTags(); }

 signals:
  void tagsLoaded(bool isValid);

 private:
  QString operationSlug;
  std::vector<model::Tag> initialTags;

  QNetworkReply* getTagsReply = nullptr;
  QNetworkReply* createTagReply = nullptr;
  std::unordered_map<QString, QNetworkReply*> activeRequests;
  TagCache* tagCache = nullptr;

  QErrorMessage* couldNotCreateTagMsg = nullptr;

  TaggingLineEditEventFilter filter;
  QCompleter* completer;
  QStringList tagNames;
  std::unordered_map<QString, dto::Tag> tagMap;

  // Ui Elements
  QGridLayout* gridLayout = nullptr;
  QLineEdit* tagCompleteTextBox = nullptr;
  QProgressIndicator* loading = nullptr;
  QLabel* errorLabel = nullptr;
  TagView* tagView = nullptr;
};

#endif  // TAGEDITOR_H
