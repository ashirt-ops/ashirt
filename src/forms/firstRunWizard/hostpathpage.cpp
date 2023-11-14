#include "hostpathpage.h"

#include <QFileDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <appconfig.h>

bool HostPathPage::validatePage()
{
  return field("host.url").isValid();
}

void HostPathPage::initializePage()
{
  QString hostPath = AppConfig::value(CONFIG::APIURL);
  setField("host.url", hostPath);
  hostPathLine->setText(hostPath);
}

HostPathPage::HostPathPage(QWidget *parent)
    : WizardPage{Page_HostPath, parent}
{
    auto f = font();
    auto _lblTitleLabel = new QLabel(this);
    f.setPointSize(titleFont.first);
    f.setWeight(titleFont.second);
    _lblTitleLabel->setFont(f);
    _lblTitleLabel->setText(tr("Server URL"));

    auto _lblSubtitle = new QLabel(this);
    f.setPointSize(subTitleFont.first);
    f.setWeight(subTitleFont.second);
    _lblSubtitle->setFont(f);
    _lblSubtitle->setWordWrap(true);
    _lblSubtitle->setText(tr("The URL of the ashirt server. Example below:"));

    auto _lblSubtitle2 = new QLabel(this);
    f.setPointSize(subTitleFont.first);
    f.setWeight(subTitleFont.second);
    f.setItalic(true);
    _lblSubtitle2->setFont(f);
    _lblSubtitle2->setWordWrap(true);
    _lblSubtitle2->setText(tr("http://localhost:8080"));

    auto _lblBody = new QLabel(this);
    f.setPointSize(bodyFont.first);
    f.setWeight(bodyFont.second);
    f.setItalic(false);
    _lblBody->setFont(f);
    _lblBody->setWordWrap(true);
    _lblBody->setText(tr("• Enter the URL that was provided by admin"));

    auto _lblPath = new QLabel(this);
    f.setPointSize(smallFont.first);
    f.setWeight(smallFont.second);
    _lblPath->setFont(f);
    _lblPath->setText(tr("Server URL:"));

    hostPathLine = new QLineEdit(this);
    hostPathLine->setTextMargins(3,0,3,0);
    registerField("host.url*", hostPathLine);

    auto t1 = new QVBoxLayout();
    t1->setSpacing(1);
    t1->addWidget(_lblPath);
    t1->addWidget(hostPathLine);

    auto layout = new QVBoxLayout();
    layout->addWidget(_lblTitleLabel);
    layout->addWidget(_lblSubtitle);
    layout->addWidget(_lblSubtitle2);
     layout->addSpacerItem(new QSpacerItem(0,30,QSizePolicy::Minimum, QSizePolicy::Fixed));
    layout->addWidget(_lblBody);
    layout->addSpacerItem(new QSpacerItem(0,60,QSizePolicy::Minimum, QSizePolicy::Fixed));
    layout->addLayout(t1);
    layout->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Minimum, QSizePolicy::Expanding));

    setLayout(layout);
}
