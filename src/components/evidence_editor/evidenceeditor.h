// Copyright 2020, Verizon Media
// Licensed under the terms of GPLv3. See LICENSE file in project root for terms.

#ifndef EVIDENCEEDITOR_H
#define EVIDENCEEDITOR_H

#include <QGridLayout>
#include <QLabel>
#include <QString>
#include <QTextEdit>
#include <QWidget>
#include <QSplitter>

#include "components/evidencepreview.h"
#include "db/databaseconnection.h"
#include "deleteevidenceresponse.h"
#include "saveevidenceresponse.h"

#include "components/tagging/tageditor.h"


class EvidenceEditor : public QWidget {
  Q_OBJECT

 public:
  explicit EvidenceEditor(qint64 evidenceID, DatabaseConnection* db, QWidget* parent = nullptr);
  explicit EvidenceEditor(DatabaseConnection* db, QWidget* parent = nullptr);
  ~EvidenceEditor();

 private:
  void buildUi();
  void wireUi();
  void loadData();
  void clearEditor();

 public:
  model::Evidence encodeEvidence();
  void setEnabled(bool enable);
  SaveEvidenceResponse saveEvidence();
  DeleteEvidenceResponse deleteEvidence();

 signals:
  void onWidgetReady();

 public slots:
  void updateEvidence(qint64 evidenceID, bool readonly);

 private slots:
  void onTagsLoaded(bool success);

 private:
  DatabaseConnection* db;
  qint64 evidenceID = 0;
  QString operationSlug;
  bool readonly = false;

  model::Evidence originalEvidenceData;

  // UI components
  QGridLayout* gridLayout;
  QSplitter* splitter;
  QLabel* _descriptionLabel;
  QTextEdit* descriptionTextBox;
  TagEditor* tagEditor;
  EvidencePreview* loadedPreview = nullptr;
};

#endif  // EVIDENCEEDITOR_H
