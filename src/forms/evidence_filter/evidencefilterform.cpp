#include "evidencefilterform.h"

#include <QComboBox>
#include <QCheckBox>
#include <QDateEdit>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QLabel>

#include "appconfig.h"
#include "helpers/netman.h"
#include "helpers/ui_helpers.h"

void EvidenceFilterForm::initializeTriCombobox(QComboBox *box) {
    box->clear();
    box->addItem(tr("Any"));
    box->addItem(tr("Yes"));
    box->addItem(tr("No"));
}

void EvidenceFilterForm::initializeDateEdit(QDateEdit *dateEdit) {
  dateEdit->setDate(QDateTime::currentDateTime().date());
  dateEdit->setDisplayFormat(QStringLiteral("MMM dd, yyyy"));
  dateEdit->setDateRange(QDate(2000, 01, 01), QDateTime::currentDateTime().date());
  dateEdit->setEnabled(false);
}

void EvidenceFilterForm::dateNormalize(bool isCondition)
{
    // swap dates so smaller date is always "from" / after
    if (isCondition && fromDateEdit->date() > toDateEdit->date()) {
      auto copy = fromDateEdit->date();
      fromDateEdit->setDate(toDateEdit->date());
      toDateEdit->setDate(copy);
    }
}

EvidenceFilterForm::EvidenceFilterForm(QWidget *parent)
    : AShirtDialog(parent)
    , operationComboBox(new QComboBox(this))
    , submittedComboBox(new QComboBox(this))
    , erroredComboBox(new QComboBox(this))
    , contentTypeComboBox(new QComboBox(this))
    , fromDateEdit(new QDateEdit(this))
    , toDateEdit(new QDateEdit(this))
    , includeStartDateCheckBox(new QCheckBox(tr("From Date"), this))
    , includeEndDateCheckBox(new QCheckBox(tr("To Date"), this))
    , buttonBox(new QDialogButtonBox(QDialogButtonBox::Ok, this))
{
  buildUi();
  wireUi();
}

void EvidenceFilterForm::buildUi() {
  erroredComboBox->setEditable(false);
  operationComboBox->setEditable(false);
  operationComboBox->setEnabled(false);
  operationComboBox->addItem(tr("Loading..."));

  submittedComboBox->setEditable(false);
  initializeTriCombobox(submittedComboBox);
  initializeTriCombobox(erroredComboBox);

  contentTypeComboBox->setEditable(false);
  contentTypeComboBox->addItem(tr("<None>"));
  contentTypeComboBox->addItem(tr("Image"), QStringLiteral("image"));
  contentTypeComboBox->addItem(tr("Codeblock"), QStringLiteral("codeblock"));

  initializeDateEdit(fromDateEdit);
  initializeDateEdit(toDateEdit);

  // Layout
  /*        0                 1           2
       +---------------+-------------+--------------+
    0  | Op  Lbl       | Op CB                      |
       +---------------+-------------+--------------+
    1  | C. Type Lbl   | Content Type CB            |
       +---------------+-------------+--------------+
    2  | hadErr Lbl    | had Error CB               |
       +---------------+-------------+--------------+
    3  | Submit Lbl    | Was Submitted CB           |
       +---------------+-------------+--------------+
    4  | From Lbl      | From DtSel  | incl From CB |
       +---------------+-------------+--------------+
    5  | To Lbl        | To DtSel    | incl To CB   |
       +---------------+-------------+--------------+
    6  | Dialog button Box{ok}                      |
       +---------------+-------------+--------------+
  */

  auto gridLayout = new QGridLayout(this);
  gridLayout->addWidget(new QLabel(tr("Operation"), this), 0, 0);
  gridLayout->addWidget(operationComboBox, 0, 1, 1, 2);

  gridLayout->addWidget(new QLabel(tr("Content Type"), this), 1, 0);
  gridLayout->addWidget(contentTypeComboBox, 1, 1, 1, 2);

  gridLayout->addWidget(new QLabel(tr("Had Error"), this), 2, 0);
  gridLayout->addWidget(erroredComboBox, 2, 1, 1, 2);

  gridLayout->addWidget(new QLabel(tr("Was Submitted"), this), 3, 0);
  gridLayout->addWidget(submittedComboBox, 3, 1, 1, 2);

  gridLayout->addWidget(includeStartDateCheckBox, 4, 0, Qt::AlignLeft);
  gridLayout->addWidget(fromDateEdit, 4, 1, 1, 2);

  gridLayout->addWidget(includeEndDateCheckBox, 5, 0, Qt::AlignLeft);
  gridLayout->addWidget(toDateEdit, 5, 1, 1, 2);

  gridLayout->addWidget(buttonBox, 6, 0, 1, gridLayout->columnCount());

  setLayout(gridLayout);
  setWindowTitle(tr("Evidence Filters"));
  resize(320, 245);
}

void EvidenceFilterForm::wireUi() {
  erroredComboBox->installEventFilter(this);
  operationComboBox->installEventFilter(this);
  submittedComboBox->installEventFilter(this);
  contentTypeComboBox->installEventFilter(this);

  connect(NetMan::get(), &NetMan::operationListUpdated, this, &EvidenceFilterForm::onOperationListUpdated);
  connect(buttonBox, &QDialogButtonBox::accepted, this, &EvidenceFilterForm::writeAndClose);

  connect(includeStartDateCheckBox, &QCheckBox::stateChanged, fromDateEdit, &QDateEdit::setEnabled);
  connect(includeEndDateCheckBox, &QCheckBox::stateChanged, toDateEdit, &QDateEdit::setEnabled);
}

void EvidenceFilterForm::writeAndClose() {
  writeForm();
  close();
}

void EvidenceFilterForm::writeForm() {
  auto filter = encodeForm();
  Q_EMIT evidenceSet(filter);
}

EvidenceFilters EvidenceFilterForm::encodeForm() {
  EvidenceFilters filter;

  filter.hasError = EvidenceFilters::parseTri(erroredComboBox->currentText());
  filter.submitted = EvidenceFilters::parseTri(submittedComboBox->currentText());
  filter.operationSlug = operationComboBox->currentData().toString();
  filter.contentType = contentTypeComboBox->currentData().toString();

  dateNormalize(fromDateEdit->isEnabled() && toDateEdit->isEnabled());

  if (includeStartDateCheckBox->isChecked()) {
    filter.startDate = fromDateEdit->date();
  }
  if (includeEndDateCheckBox->isChecked()) {
    filter.endDate = toDateEdit->date();
  }

  return filter;
}

void EvidenceFilterForm::setForm(const EvidenceFilters &model) {
  UIHelpers::setComboBoxValue(operationComboBox, model.operationSlug);
  UIHelpers::setComboBoxValue(contentTypeComboBox, model.contentType);
  erroredComboBox->setCurrentText(EvidenceFilters::triToString(model.hasError));
  submittedComboBox->setCurrentText(EvidenceFilters::triToString(model.submitted));

  includeStartDateCheckBox->setChecked(model.startDate.isValid());
  fromDateEdit->setDate(model.startDate.isValid() ? model.startDate
                                                      : QDateTime::currentDateTime().date());

  includeEndDateCheckBox->setChecked(model.endDate.isValid());
  toDateEdit->setDate(model.endDate.isValid() ? model.endDate
                                                  : QDateTime::currentDateTime().date());

  dateNormalize(model.startDate.isValid() && model.endDate.isValid());
}

void EvidenceFilterForm::onOperationListUpdated(bool success,
                                                const QList<dto::Operation> &operations) {
  operationComboBox->setEnabled(false);
  if (!success) {
    operationComboBox->setItemText(0, tr("Unable to fetch operations"));
    operationComboBox->setCurrentIndex(0);
    return;
  }

  operationComboBox->clear();
  operationComboBox->addItem(tr("<None>"));
  for (const auto &op : operations) {
    operationComboBox->addItem(op.name, op.slug);
  }
  UIHelpers::setComboBoxValue(operationComboBox, AppConfig::operationSlug());
  operationComboBox->setEnabled(true);
}
