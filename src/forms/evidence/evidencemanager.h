// Copyright 2020, Verizon Media
// Licensed under the terms of GPLv3. See LICENSE file in project root for terms.

#ifndef EVIDENCEMANAGER_H
#define EVIDENCEMANAGER_H

#include <QDialog>
#include <QKeyEvent>
#include <QNetworkReply>
#include <QTableWidgetItem>

#include "components/evidence_editor/evidenceeditor.h"
#include "components/loading_button/loadingbutton.h"
#include "db/databaseconnection.h"
#include "forms/evidence_filter/evidencefilterform.h"

namespace Ui {
class EvidenceManager;
}

// QTableWidget should memory-manage this data.
struct EvidenceRow {
  QTableWidgetItem* dateCaptured;
  QTableWidgetItem* description;
  QTableWidgetItem* contentType;
  QTableWidgetItem* operation;
  QTableWidgetItem* submitted;
  QTableWidgetItem* failed;
  QTableWidgetItem* path;
  QTableWidgetItem* errorText;
  QTableWidgetItem* dateSubmitted;
};

class EvidenceManager : public QDialog {
  Q_OBJECT

 public:
  explicit EvidenceManager(DatabaseConnection* db, QWidget* parent = nullptr);
  ~EvidenceManager();

 protected:
  void closeEvent(QCloseEvent* event) override;

 private:
  void wireUi();

  bool saveData();
  void loadEvidence();
  void setActionButtonsEnabled(bool enabled);
  EvidenceRow buildBaseEvidenceRow(qint64 evidenceID);
  void refreshRow(int row);
  void setRowText(int row, const model::Evidence& model);
  void enableEvidenceButtons(bool enable);

  void showEvent(QShowEvent* evt) override;
  void keyPressEvent(QKeyEvent *evt) override;
  qint64 selectedRowEvidenceID();

 signals:
  void evidenceChanged(quint64 evidenceID, bool readonly);

 private slots:
  void submitEvidenceButtonClicked();
  void deleteEvidenceButtonClicked();
  void applyFilterButtonClicked();
  void resetFilterButtonClicked();
  void applyFilterForm(const EvidenceFilters& filter);
  void openFiltersMenu();

  void onRowChanged(int currentRow, int currentColumn, int previousRow, int previousColumn);
  void onUploadComplete();

 private:
  Ui::EvidenceManager* ui;
  EvidenceEditor* evidenceEditor;
  EvidenceFilterForm* filterForm;
  LoadingButton* submitButton;

  DatabaseConnection* db;

  QNetworkReply* uploadAssetReply = nullptr;
  qint64 evidenceIDForRequest;
};

#endif  // EVIDENCEMANAGER_H
