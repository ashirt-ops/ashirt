#include "finishedpage.h"

#include <QDesktopServices>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <appconfig.h>

void FinishedPage::initializePage()
{
  auto shortCut = QKeySequence(AppConfig::value(CONFIG::SHORTCUT_CAPTUREWINDOW)).toString(QKeySequence::NativeText);
  m_lblBody->setText(tr("\n\nYou can now submit evidence Try it now,\n Use %1 to capture the window").arg(shortCut));
}

bool FinishedPage::validatePage()
{
  AppConfig::setValue(CONFIG::SHOW_WELCOME_SCREEN, QStringLiteral("false"));
  return true;
}

FinishedPage::FinishedPage(QWidget *parent)
    : WizardPage{Page_Finished, parent}
    , m_lblBody{new QLabel(this)}
{
  auto f = font();
  auto _lblTitleLabel = new QLabel(this);
  f.setPointSize(titleFont.first);
  f.setWeight(titleFont.second);
  _lblTitleLabel->setFont(f);
  _lblTitleLabel->setText(tr("Congratulations!"));

  auto _lblSubtitle = new QLabel(this);
  f.setPointSize(subTitleFont.first);
  f.setWeight(subTitleFont.second);
  _lblSubtitle->setFont(f);
  _lblSubtitle->setWordWrap(true);
  _lblSubtitle->setText(tr("You have successfully completed the configuration setup and can continue working within the ashirt application as needed."));

  f.setPointSize(bodyFont.first);
  f.setWeight(bodyFont.second);
  f.setItalic(true);
  m_lblBody->setFont(f);

  auto _lblBody2 = new QLabel(this);
  f.setPointSize(bodyFont.first);
  f.setWeight(bodyFont.second);
  f.setItalic(true);
  _lblBody2->setFont(f);
  _lblBody2->setText(tr("\n\nFor more information or support"));

  QString buttonStyle = QStringLiteral("QPushButton:enabled{background-color: rgba(93, 172, 232, 255);font:;color:\"black\";}");
  auto _faqButton = new QPushButton(this);
  _faqButton->setText(tr("Click here"));
  _faqButton->setStyleSheet(buttonStyle);
  connect(_faqButton, &QPushButton::clicked, this, [this]{
    QDesktopServices::openUrl(QUrl(QStringLiteral("https://ashirt.io"), QUrl::TolerantMode));
  });

  auto hLayout = new QHBoxLayout();
  hLayout->addWidget(_lblBody2, 8, Qt::AlignLeft | Qt::AlignBottom);
  hLayout->addWidget(_faqButton, 2, Qt::AlignLeft | Qt::AlignBottom);

  auto layout = new QVBoxLayout();
  layout->addWidget(_lblTitleLabel);
  layout->addWidget(_lblSubtitle);
  layout->addWidget(m_lblBody);
  layout->addLayout(hLayout);
  layout->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Minimum, QSizePolicy::Expanding));
  setLayout(layout);
}
