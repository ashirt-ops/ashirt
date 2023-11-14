#include "opspage.h"

#include <QNetworkReply>
#include <QComboBox>
#include <QFileDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <appconfig.h>

#include "helpers/netman.h"

#include <dtos/ashirt_error.h>

bool OpsPage::validatePage()
{
  if (!field("ops").isValid())
    return false;

  QString slug = opsCombo->currentData().toString();
  QString name = opsCombo->currentText();

  if(!_OpsList.contains(name)) {
    slug = name.toLower().replace(invalidCharsRegex, QStringLiteral("-")).replace(startOrEndDash, QString());
    createOpReply = NetMan::createOperation(name, slug);
    connect(createOpReply, &QNetworkReply::finished, this, &OpsPage::createOpComplete);
    return successful;
  }
  AppConfig::setOperationDetails(slug, name);
  return true;
}

void OpsPage::initializePage()
{
  NetMan::refreshOperationsList();
  int v = opsCombo->findText(AppConfig::operationName());
  setField("ops", v);
  opsCombo->setCurrentIndex(v);
}

OpsPage::OpsPage(QWidget *parent)
    : WizardPage{Page_Ops, parent}
{
      connect(NetMan::get(), &NetMan::operationListUpdated, this, &OpsPage::operationsUpdated);
      auto f = font();

      auto _lblTitleLabel = new QLabel(this);
      f.setPointSize(titleFont.first);
      f.setWeight(titleFont.second);
      _lblTitleLabel->setFont(f);
      _lblTitleLabel->setText(tr("Select or Create Operation"));

      auto _lblSubtitle = new QLabel(this);
      f.setPointSize(subTitleFont.first);
      f.setWeight(subTitleFont.second);
      _lblSubtitle->setFont(f);
      _lblSubtitle->setWordWrap(true);
      _lblSubtitle->setText(tr("Select the desired operation below. If none exist you can create a new one."));

      auto _lblBody = new QLabel(this);
      f.setPointSize(bodyFont.first);
      f.setWeight(bodyFont.second);
      _lblBody->setFont(f);
      _lblBody->setWordWrap(true);
      _lblBody->setText(tr("• Select the desired operation below. If none exist you can create a new one."));

      auto _lblBody2 = new QLabel(this);
      f.setPointSize(bodyFont.first);
      f.setWeight(bodyFont.second);
      _lblBody2->setFont(f);
      _lblBody2->setWordWrap(true);
      _lblBody2->setText(tr("• The user that creates an operation automatically becomes the admin, and can assign access to additional users in the ashirt server."));

      auto _lblEvidence = new QLabel(this);
      f.setPointSize(smallFont.first);
      f.setWeight(smallFont.second);
      _lblEvidence->setFont(f);
      _lblEvidence->setText(tr("Operation Name:"));

      opsCombo = new QComboBox(this);
      registerField("ops", opsCombo);

      responseLabel = new QLabel(this);
      f.setPointSize(smallFont.first);
      f.setWeight(smallFont.second);
      responseLabel->setFont(f);

      auto t1 = new QVBoxLayout();
      t1->setSpacing(1);
      t1->addWidget(_lblEvidence);
      t1->addWidget(opsCombo);
      t1->addWidget(responseLabel);

      auto topLayout = new QVBoxLayout();
      topLayout->addWidget(_lblTitleLabel);
      topLayout->addWidget(_lblSubtitle);
      topLayout->addSpacerItem(new QSpacerItem(0,20,QSizePolicy::Minimum, QSizePolicy::Fixed));
      topLayout->addWidget(_lblBody);
      topLayout->addSpacerItem(new QSpacerItem(0,10,QSizePolicy::Minimum, QSizePolicy::Fixed));
      topLayout->addWidget(_lblBody2);
      topLayout->addSpacerItem(new QSpacerItem(0,50,QSizePolicy::Minimum, QSizePolicy::Fixed));
      topLayout->addLayout(t1);
      topLayout->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Minimum, QSizePolicy::Expanding));
      setLayout(topLayout);
}

void OpsPage::operationsUpdated(bool success, const QList<dto::Operation> & operations)
{
  if (!success)
      return;

  opsCombo->clear();
  opsCombo->setEditable(true);
  _OpsList.clear();
  for (const auto& op : operations) {
    _OpsList.insert(op.name, op.slug);
    opsCombo->addItem(op.name, op.slug);
  }
  int v = opsCombo->findText(AppConfig::operationName());
  setField("ops", v);
  opsCombo->setCurrentIndex(v);
}

void OpsPage::createOpComplete()
{
  bool isValid;
  auto data = NetMan::extractResponse(createOpReply, isValid);
  if (isValid) {
    dto::Operation op = dto::Operation::parseData(data);
    AppConfig::setOperationDetails(op.slug, op.name);
    NetMan::refreshOperationsList();
    cleanUpReply(&createOpReply);
    successful = true;
    wizard()->next();
  } else {
    setField("ops", -1);
    successful = false;
    cleanUpReply(&createOpReply);
    dto::AShirtError err = dto::AShirtError::parseData(data);
    if (err.error.contains(QStringLiteral("slug already exists"))) {
      responseLabel->setText(tr("A similar operation name already exists. Please try a new name."));
    }
    else {
      responseLabel->setText(tr("Got an unexpected error: %1").arg(err.error));
    }
  }
}
