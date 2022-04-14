// Copyright 2020, Verizon Media
// Licensed under the terms of MIT. See LICENSE file in project root for terms.

#ifndef EVIDENCEFILTERFORM_H
#define EVIDENCEFILTERFORM_H

#include <QComboBox>
#include <QDialog>
#include <QAction>
#include <QGridLayout>
#include <QLabel>
#include <QComboBox>
#include <QDateEdit>
#include <QCheckBox>
#include <QDialogButtonBox>

#include "db/databaseconnection.h"
#include "dtos/operation.h"

class EvidenceFilterForm : public QDialog {
  Q_OBJECT

 public:
  explicit EvidenceFilterForm(QWidget *parent = nullptr);
  ~EvidenceFilterForm();

 private:
  /// buildUi creates the window structure.
  void buildUi();
  /// wireUi connects the components to each other.
  void wireUi();
  /// writeForm encodes the current form and sends a modified event for listeners
  void writeForm();
  /// writeAndClose is shorthand for WriteForm and close the window
  void writeAndClose();

 public:
  /// setForm updates the editor to match the provided filter model
  void setForm(const EvidenceFilters &model);

 signals:
  /// evidenceSet alerts listeners when the form has been "saved" by the user
  void evidenceSet(EvidenceFilters filter);

 public slots:
  /// onOperationListUpdated listens for events to know when to update the operations checkbox
  void onOperationListUpdated(bool success, const std::vector<dto::Operation> &operations);

  /// encodeForm converts the current form into a EvidenceFilter model
  EvidenceFilters encodeForm();

 private:
  QAction* closeWindowAction = nullptr;

  // UI Components
  QGridLayout* gridLayout = nullptr;
  QLabel* _operationLabel = nullptr;
  QLabel* _contentTypeLabel = nullptr;
  QLabel* _hadErrorLabel = nullptr;
  QLabel* _wasSubmittedLabel = nullptr;
  QLabel* _fromDateLabel = nullptr;
  QLabel* _toDateLabel = nullptr;

  QComboBox* operationComboBox = nullptr;
  QComboBox* submittedComboBox = nullptr;
  QComboBox* erroredComboBox = nullptr;
  QComboBox* contentTypeComboBox = nullptr;
  QDateEdit* fromDateEdit = nullptr;
  QDateEdit* toDateEdit = nullptr;
  QCheckBox* includeEndDateCheckBox = nullptr;
  QCheckBox* includeStartDateCheckBox = nullptr;
  QDialogButtonBox* buttonBox = nullptr;
};

#endif  // EVIDENCEFILTERFORM_H
