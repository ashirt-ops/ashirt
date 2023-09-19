#include "requirementspage.h"

#include <QLabel>
#include <QVBoxLayout>

RequirementsPage::RequirementsPage(QWidget *parent)
    : WizardPage{Page_Requiments, parent}
{
    auto f = font();

    auto _lblTitleLabel = new QLabel(this);
    f.setPointSize(titleFont.first);
    f.setWeight(titleFont.second);
    _lblTitleLabel->setFont(f);
    _lblTitleLabel->setText(tr("Requirements"));

    auto _lblSubtitle = new QLabel(this);
    f.setPointSize(subTitleFont.first);
    f.setWeight(subTitleFont.second);
    _lblSubtitle->setFont(f);
    _lblSubtitle->setWordWrap(true);
    _lblSubtitle->setText(tr("To get started, make sure you have acquired the following information from your admin:"));

    auto _lblBody = new QLabel(this);
    f.setPointSize(bodyFont.first);
    f.setWeight(bodyFont.second);
    _lblBody->setFont(f);
    _lblBody->setText(tr("\n\n\n\t• URL for the ashirt server\n\n\t• Login method"));

    auto topLayout = new QVBoxLayout();
    topLayout->addWidget(_lblTitleLabel);
    topLayout->addWidget(_lblSubtitle);
    topLayout->addWidget(_lblBody);
    topLayout->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Minimum, QSizePolicy::Expanding));
    setLayout(topLayout);
}
