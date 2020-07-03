// Copyright 2020, Verizon Media
// Licensed under the terms of GPLv3. See LICENSE file in project root for terms.

#include "evidencefilterform.h"

#include <QKeySequence>

#include "appsettings.h"
#include "helpers/netman.h"
#include "helpers/ui_helpers.h"
#include "ui_evidencefilterform.h"

static void initializeTriCombobox(QComboBox *box) {
  box->clear();
  box->addItem("Any");
  box->addItem("Yes");
  box->addItem("No");
}

static void initializeDateEdit(QDateEdit *dateEdit) {
  dateEdit->setDate(QDateTime::currentDateTime().date());
  dateEdit->setDisplayFormat("MMM dd, yyyy");
  dateEdit->setDateRange(QDate(2000, 01, 01), QDateTime::currentDateTime().date());
  dateEdit->setEnabled(false);
}

EvidenceFilterForm::EvidenceFilterForm(QWidget *parent)
    : QDialog(parent), ui(new Ui::EvidenceFilterForm) {
  ui->setupUi(this);

  ui->erroredComboBox->setEditable(false);
  ui->operationComboBox->setEditable(false);
  ui->submittedComboBox->setEditable(false);
  ui->contentTypeComboBox->setEditable(false);

  initializeTriCombobox(ui->submittedComboBox);
  initializeTriCombobox(ui->erroredComboBox);
  ui->contentTypeComboBox->addItem("<None>", "");
  ui->contentTypeComboBox->addItem("Image", "image");
  ui->contentTypeComboBox->addItem("Codeblock", "codeblock");

  initializeDateEdit(ui->fromDateEdit);
  initializeDateEdit(ui->toDateEdit);

  closeWindowAction = new QAction(this);
  closeWindowAction->setShortcut(QKeySequence::Close);
  this->addAction(closeWindowAction);

  wireUi();
}

EvidenceFilterForm::~EvidenceFilterForm() {
  delete ui;
  delete closeWindowAction;
}

void EvidenceFilterForm::wireUi() {
  ui->erroredComboBox->installEventFilter(this);
  ui->operationComboBox->installEventFilter(this);
  ui->submittedComboBox->installEventFilter(this);
  ui->contentTypeComboBox->installEventFilter(this);

  connect(&NetMan::getInstance(), &NetMan::operationListUpdated, this,
          &EvidenceFilterForm::onOperationListUpdated);
  connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &EvidenceFilterForm::writeForm);

  connect(ui->includeStartDateCheckBox, &QCheckBox::stateChanged,
          [this](bool checked) { ui->fromDateEdit->setEnabled(checked); });
  connect(ui->includeEndDateCheckBox, &QCheckBox::stateChanged,
          [this](bool checked) { ui->toDateEdit->setEnabled(checked); });

  connect(closeWindowAction, &QAction::triggered, [this](){writeForm(); close();});
}

void EvidenceFilterForm::writeForm() {
  auto filter = encodeForm();
  emit evidenceSet(filter);
}

EvidenceFilters EvidenceFilterForm::encodeForm() {
  EvidenceFilters filter;

  filter.hasError = EvidenceFilters::parseTri(ui->erroredComboBox->currentText());
  filter.submitted = EvidenceFilters::parseTri(ui->submittedComboBox->currentText());
  filter.operationSlug = ui->operationComboBox->currentData().toString();
  filter.contentType = ui->contentTypeComboBox->currentData().toString();

  // swap dates so smaller date is always "from" / after
  if (ui->fromDateEdit->isEnabled() && ui->toDateEdit->isEnabled() &&
      ui->fromDateEdit->date() > ui->toDateEdit->date()) {
    auto copy = ui->fromDateEdit->date();
    ui->fromDateEdit->setDate(ui->toDateEdit->date());
    ui->toDateEdit->setDate(copy);
  }

  if (ui->includeStartDateCheckBox->isChecked()) {
    filter.startDate = ui->fromDateEdit->date();
  }
  if (ui->includeEndDateCheckBox->isChecked()) {
    filter.endDate = ui->toDateEdit->date();
  }

  return filter;
}

void EvidenceFilterForm::setForm(const EvidenceFilters &model) {
  UiHelpers::setComboBoxValue(ui->operationComboBox, model.operationSlug);
  UiHelpers::setComboBoxValue(ui->contentTypeComboBox, model.contentType);
  ui->erroredComboBox->setCurrentText(EvidenceFilters::triToString(model.hasError));
  ui->submittedComboBox->setCurrentText(EvidenceFilters::triToString(model.submitted));

  ui->includeStartDateCheckBox->setChecked(model.startDate.isValid());
  ui->fromDateEdit->setDate(model.startDate.isValid() ? model.startDate
                                                      : QDateTime::currentDateTime().date());

  ui->includeEndDateCheckBox->setChecked(model.endDate.isValid());
  ui->toDateEdit->setDate(model.endDate.isValid() ? model.endDate
                                                  : QDateTime::currentDateTime().date());

  // swap dates so smaller date is always "from" / after
  if (model.startDate.isValid() && model.endDate.isValid() &&
      ui->fromDateEdit->date() > ui->toDateEdit->date()) {
    auto copy = ui->fromDateEdit->date();
    ui->fromDateEdit->setDate(ui->toDateEdit->date());
    ui->toDateEdit->setDate(copy);
  }
}

void EvidenceFilterForm::onOperationListUpdated(bool success,
                                                const std::vector<dto::Operation> &operations) {
  ui->operationComboBox->setEnabled(false);
  if (!success) {
    ui->operationComboBox->setItemText(0, "Unable to fetch operations");
    ui->operationComboBox->setCurrentIndex(0);
    return;
  }

  ui->operationComboBox->clear();
  ui->operationComboBox->addItem("<None>", "");
  for (const auto &op : operations) {
    ui->operationComboBox->addItem(op.name, op.slug);
  }
  UiHelpers::setComboBoxValue(ui->operationComboBox, AppSettings::getInstance().operationSlug());
  ui->operationComboBox->setEnabled(true);
}
