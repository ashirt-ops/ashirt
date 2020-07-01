// Copyright 2020, Verizon Media
// Licensed under the terms of GPLv3. See LICENSE file in project root for terms.

#ifndef EVIDENCEMANAGER_H
#define EVIDENCEMANAGER_H

#include <QDialog>
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

/**
 * @brief The EvidenceManager class represents the Evidence Manager window that is shown
 * when selecting "View Accumulated Evidence."
 */
class EvidenceManager : public QDialog {
  Q_OBJECT

 public:
  explicit EvidenceManager(DatabaseConnection* db, QWidget* parent = nullptr);
  ~EvidenceManager();

 protected:
  /// closeEvent extends QDialog's closeEvent. Clears the evidenceEditor after closing.
  void closeEvent(QCloseEvent* event) override;

 private:
  /// wireUi connects UI elements together
  void wireUi();

  /// saveData stores any edits in evidence view. Deprecated (edits no longer available)
  bool saveData();
  /// loadEvidence retrieves data from the database and renders the evidence table
  void loadEvidence();
  /// setActionButtonsEnabled enables the delete/submit ui buttons
  void setActionButtonsEnabled(bool enabled);
  /// buildBaseEvidenceRow constructs a basic evidence row (fields and formatting, no data applied)
  EvidenceRow buildBaseEvidenceRow(qint64 evidenceID);
  /// refreshRow updates the indicated row (0-based) with updated (database) data.
  void refreshRow(int row);
  /// setRowText writes data the indicated row (0-based) based on the given model
  void setRowText(int row, const model::Evidence& model);
  ///enableEvidenceButtons enables the delete/submit ui buttons
  void enableEvidenceButtons(bool enable);

  /// showEvent extends QDialog's showEvent. Resets the applied filters.
  void showEvent(QShowEvent* evt) override;
  /// selectedRowEvidenceID is a small helper to get the evidence id for the currently selected row.
  qint64 selectedRowEvidenceID();

 signals:
  /**
   * @brief evidenceChanged is emitted when a user changes the selection in the evidence table
   * @param evidenceID the evidence ID of the now-selected evidence
   * @param readonly True if this evidence can be edited. False otherwise.
   */
  void evidenceChanged(quint64 evidenceID, bool readonly);

 private slots:
  /// submitEvidenceButtonClicked recieves the submit button clicked event
  void submitEvidenceButtonClicked();
  /// deleteEvidenceButtonClicked recieves the delete button clicked event
  void deleteEvidenceButtonClicked();
  /// applyFilterButtonClicked recieves the apply filter button clicked event
  void applyFilterButtonClicked();
  /// resetFilterButtonClicked recieves the reset filter button clicked event
  void resetFilterButtonClicked();
  /// applyFilterForm updates the filter textbox to reflect the filter options chosen in the filter
  /// menu
  void applyFilterForm(const EvidenceFilters& filter);
  /// openFiltersMenu opens the filter menu with the current filters applied
  void openFiltersMenu();

  /// onRowChanged recieves the event from the evidence table rowChange signal
  void onRowChanged(int currentRow, int currentColumn, int previousRow, int previousColumn);
  /// onUploadComplete is triggered when the upload response has been received.
  void onUploadComplete();

 private:
  Ui::EvidenceManager* ui;

  /// db is a (shared) reference to the local database instance. Not to be deleted.
  DatabaseConnection* db;

  QNetworkReply* uploadAssetReply = nullptr;
  qint64 evidenceIDForRequest;

  // UI Elements
  EvidenceEditor* evidenceEditor;
  EvidenceFilterForm* filterForm;
  LoadingButton* submitButton;

};

#endif  // EVIDENCEMANAGER_H
