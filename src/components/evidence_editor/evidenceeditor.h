#pragma once

#include <QWidget>

#include "deleteevidenceresponse.h"
#include "saveevidenceresponse.h"

class QSplitter;
class QTextEdit;
class TagEditor;
class EvidencePreview;
class DatabaseConnection;

class EvidenceEditor : public QWidget {
  Q_OBJECT

 public:
  explicit EvidenceEditor(qint64 evidenceID, DatabaseConnection* db, QWidget* parent = nullptr);
  explicit EvidenceEditor(DatabaseConnection* db, QWidget* parent = nullptr);
  ~EvidenceEditor() = default;

 private:
  void buildUi();
  void loadData();
  void clearEditor();

 public:
  model::Evidence encodeEvidence();
  void setEnabled(bool enable);
  SaveEvidenceResponse saveEvidence();

  /// deleteEvidence is a helper method to delete both the database record and
  /// file location of the provided evidence IDs
  QList<DeleteEvidenceResponse> deleteEvidence(QList<qint64> evidenceIDs);

  /// revert re-loads the evidence to restore the content to the saved version.
  /// Only useful when used in the evidence manager.
  void revert();

 signals:
  void onWidgetReady();

 public slots:
  void updateEvidence(qint64 evidenceID, bool readonly);

 private slots:
  void onTagsLoaded(bool success);

 private:
  DatabaseConnection* db = nullptr;
  qint64 evidenceID = 0;
  QString operationSlug;
  bool readonly = false;

  model::Evidence originalEvidenceData;

  // UI components
  QSplitter* splitter = nullptr;
  QTextEdit* descriptionTextBox = nullptr;
  TagEditor* tagEditor = nullptr;
  EvidencePreview* loadedPreview = nullptr;
};
