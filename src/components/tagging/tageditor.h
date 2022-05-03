#pragma once

#include <QCompleter>
#include <QWidget>

#include "components/tagging/tagview.h"
#include "components/tagging/tagwidget.h"
#include "components/tagging/tagginglineediteventfilter.h"
#include "models/tag.h"


class QNetworkReply;
class QProgressIndicator;
class QLineEdit;
class TagCache;

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

  QNetworkReply* createTagReply = nullptr;
  std::unordered_map<QString, QNetworkReply*> activeRequests;
  TagCache* tagCache = nullptr;

  TaggingLineEditEventFilter filter;
  QCompleter* completer;
  QStringList tagNames;
  std::unordered_map<QString, dto::Tag> tagMap;

  // Ui Elements
  QLineEdit* tagCompleteTextBox = nullptr;
  QProgressIndicator* loading = nullptr;
  QLabel* errorLabel = nullptr;
  TagView* tagView = nullptr;
};
