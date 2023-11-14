#include "apikeyspage.h"

#include <QFileDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <appconfig.h>

bool ApiKeysPage::validatePage()
{
  if (!field("host.accessKey").isValid())
    return false;

  if (!field("host.secretKey").isValid())
    return false;

  return true;
}

void ApiKeysPage::initializePage()
{
  QString accessKey = AppConfig::value(CONFIG::ACCESSKEY);
  setField("host.accessKey", accessKey);
  accessKeyLine->setText(accessKey);

  QString secretKey = AppConfig::value(CONFIG::SECRETKEY);
  setField("host.secretKey", secretKey);
  secretKeyLine->setText(secretKey);
}

ApiKeysPage::ApiKeysPage(QWidget *parent)
    : WizardPage{Page_Api, parent}
{

  auto f = font();
  auto _lblTitleLabel = new QLabel(this);
  f.setPointSize(titleFont.first);
  f.setWeight(titleFont.second);
  _lblTitleLabel->setFont(f);
  _lblTitleLabel->setText(tr("API Keys"));

  auto _lblSubtitle = new QLabel(this);
  f.setPointSize(subTitleFont.first);
  f.setWeight(subTitleFont.second);
  _lblSubtitle->setFont(f);
  _lblSubtitle->setWordWrap(true);
  _lblSubtitle->setText(tr("Input Api keys for the server"));

  auto _lblBody = new QLabel(this);
  f.setPointSize(bodyFont.first);
  f.setWeight(bodyFont.second);
  _lblBody->setFont(f);
  _lblBody->setWordWrap(true);
  _lblBody->setText(tr("• Login to the Ashirt server click the profile icon in the top right then select <i>Account Settings</i><br>"));

  auto _lblBody2 = new QLabel(this);
  _lblBody2->setFont(f);
  _lblBody2->setWordWrap(true);
  _lblBody2->setText(tr("• Select <i>API Keys</i> on the left, select click <i>Generate New API Key</i><br>"));

  auto _lblBody3 = new QLabel(this);
  _lblBody3->setFont(f);
  _lblBody3->setWordWrap(true);
  _lblBody3->setText(tr("• Enter your keys below"));

  auto _lblAKey = new QLabel(this);
  f.setPointSize(smallFont.first);
  f.setWeight(smallFont.second);
  _lblAKey->setFont(f);
  _lblAKey->setText(tr("Access Key"));

  accessKeyLine = new QLineEdit(this);
  accessKeyLine->setMaxLength(24);
  accessKeyLine->setTextMargins(3,0,3,0);
  accessKeyLine->setMaximumWidth(fontMetrics().averageCharWidth() * 27);
  registerField("host.accessKey*", accessKeyLine);

  auto t1 = new QVBoxLayout();
  t1->setSpacing(1);
  t1->addWidget(_lblAKey);
  t1->addWidget(accessKeyLine);

  auto _lblBKey = new QLabel(this);
  f.setPointSize(smallFont.first);
  f.setWeight(smallFont.second);
  _lblBKey->setFont(f);
  _lblBKey->setText(tr("Secret Key"));

  secretKeyLine = new QLineEdit(this);
  secretKeyLine->setTextMargins(3,0,3,0);
  registerField("host.secretKey*", secretKeyLine);

  auto t2 = new QVBoxLayout();
  t2->setSpacing(1);
  t2->addWidget(_lblBKey);
  t2->addWidget(secretKeyLine);

  auto tLayout = new QHBoxLayout();
  tLayout->addLayout(t1);
  tLayout->addLayout(t2);

  auto layout = new QVBoxLayout();
  layout->addWidget(_lblTitleLabel);
  layout->addWidget(_lblSubtitle);
  layout->addSpacerItem(new QSpacerItem(0,30,QSizePolicy::Minimum, QSizePolicy::Fixed));
  layout->addWidget(_lblBody);
  layout->addWidget(_lblBody2);
  layout->addWidget(_lblBody3);
  layout->addSpacerItem(new QSpacerItem(0,30,QSizePolicy::Minimum, QSizePolicy::Fixed));
  layout->addLayout(tLayout);
  layout->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Minimum, QSizePolicy::Expanding));
  setLayout(layout);
}
