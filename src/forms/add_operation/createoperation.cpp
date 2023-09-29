#include "createoperation.h"

#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QNetworkReply>
#include <QRegularExpression>

#include "appconfig.h"
#include "dtos/ashirt_error.h"
#include "components/loading_button/loadingbutton.h"
#include "helpers/netman.h"
#include "helpers/cleanupreply.h"

CreateOperation::CreateOperation(QWidget* parent)
  : AShirtDialog(parent, AShirtDialog::commonWindowFlags)
    , submitButton(new LoadingButton(tr("Submit"), this))
    , responseLabel(new QLabel(this))
    , operationNameTextBox(new QLineEdit(this))
{
    submitButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    connect(submitButton, &QPushButton::clicked, this, &CreateOperation::submitButtonClicked);

    // Layout
    /*        0                 1            2
         +---------------+-------------+------------+
      0  | Op Lbl        | [Operation TB]           |
         +---------------+-------------+------------+
      1  | Error Lbl                                |
         +---------------+-------------+------------+
      2  | <None>        | <None>      | Submit Btn |
         +---------------+-------------+------------+
    */

    auto gridLayout = new QGridLayout(this);
    gridLayout->addWidget(new QLabel(tr("Operation Name"), this), 0, 0);
    gridLayout->addWidget(operationNameTextBox, 0, 1, 1, 2);
    gridLayout->addWidget(responseLabel, 1, 0, 1, 3);
    gridLayout->addWidget(submitButton, 2, 2);
    setLayout(gridLayout);

    resize(400, 1);
    setWindowTitle(tr("Create Operation"));
}

CreateOperation::~CreateOperation() {
  cleanUpReply(&createOpReply);
}

void CreateOperation::submitButtonClicked() {
  responseLabel->clear();
  auto name = operationNameTextBox->text().trimmed();
  auto slug = makeSlugFromName(name);

  if (slug.isEmpty()) {
    responseLabel->setText(
        (name.isEmpty())
        ? tr("The Operation Name must not be empty")
        : tr("The Operation Name must include letters or numbers")
    );
    return;
  }

  submitButton->startAnimation();
  submitButton->setEnabled(false);
  createOpReply = NetMan::createOperation(name, slug);
  connect(createOpReply, &QNetworkReply::finished, this, &CreateOperation::onRequestComplete);
}

QString CreateOperation::makeSlugFromName(QString name) {
  static QRegularExpression invalidCharsRegex(QStringLiteral("[^A-Za-z0-9]+"));
  static QRegularExpression startOrEndDash(QStringLiteral("^-|-$"));

  return name.toLower().replace(invalidCharsRegex, QStringLiteral("-")).replace(startOrEndDash, QString());
}

void CreateOperation::onRequestComplete() {
  bool isValid;
  auto data = NetMan::extractResponse(createOpReply, isValid);
  if (isValid) {
    dto::Operation op = dto::Operation::parseData(data);
    AppConfig::setOperationDetails(op.slug, op.name);
    operationNameTextBox->clear();
    NetMan::refreshOperationsList();
    close();
  }
  else {
    dto::AShirtError err = dto::AShirtError::parseData(data);
    if (err.error.contains(QStringLiteral("slug already exists"))) {
      responseLabel->setText(tr("A similar operation name already exists. Please try a new name."));
    }
    else {
      responseLabel->setText(tr("Got an unexpected error: %1").arg(err.error));
    }
  }

  submitButton->stopAnimation();
  submitButton->setEnabled(true);

  cleanUpReply(&createOpReply);
}
