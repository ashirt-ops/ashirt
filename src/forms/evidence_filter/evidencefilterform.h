#pragma once

#include "ashirtdialog/ashirtdialog.h"

#include "dtos/operation.h"
#include "evidencefilter.h"

class QComboBox;
class QLabel;
class QDateEdit;
class QCheckBox;
class QDialogButtonBox;

class EvidenceFilterForm : public AShirtDialog {
  Q_OBJECT

 public:
  explicit EvidenceFilterForm(QWidget *parent = nullptr);
  ~EvidenceFilterForm() = default;

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
  void onOperationListUpdated(bool success, const QList<dto::Operation> &operations);

  /// encodeForm converts the current form into a EvidenceFilter model
  EvidenceFilters encodeForm();

 private:
  // UI Components
  QComboBox* operationComboBox = nullptr;
  QComboBox* submittedComboBox = nullptr;
  QComboBox* erroredComboBox = nullptr;
  QComboBox* contentTypeComboBox = nullptr;
  QDateEdit* fromDateEdit = nullptr;
  QDateEdit* toDateEdit = nullptr;
  QCheckBox* includeEndDateCheckBox = nullptr;
  QCheckBox* includeStartDateCheckBox = nullptr;
  QDialogButtonBox* buttonBox = nullptr;
  void initializeTriCombobox(QComboBox *box);
  void initializeDateEdit(QDateEdit *dateEdit);
  void dateNormalize(bool isCondition = false);
};
