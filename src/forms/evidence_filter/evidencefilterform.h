// Copyright 2020, Verizon Media
// Licensed under the terms of GPLv3. See LICENSE file in project root for terms.

#ifndef EVIDENCEFILTERFORM_H
#define EVIDENCEFILTERFORM_H

#include <QComboBox>
#include <QDialog>
#include <QAction>

#include "src/db/databaseconnection.h"
#include "src/dtos/operation.h"

namespace Ui {
class EvidenceFilterForm;
}

class EvidenceFilterForm : public QDialog {
  Q_OBJECT

 public:
  explicit EvidenceFilterForm(QWidget *parent = nullptr);
  ~EvidenceFilterForm();

 private:
  void wireUi();
  void writeForm();

 public:
  void setForm(const EvidenceFilters &model);

 signals:
  void evidenceSet(EvidenceFilters filter);

 public slots:
  void onOperationListUpdated(bool success, const std::vector<dto::Operation> &operations);
  EvidenceFilters encodeForm();

 private:
  Ui::EvidenceFilterForm *ui;

  QAction* closeWindowAction = nullptr;
};

#endif  // EVIDENCEFILTERFORM_H
