#include "welcomepage.h"
#include <QCheckBox>
#include <QDesktopServices>
#include <QGridLayout>
#include <QLabel>
#include <QLine>
#include <QPainter>
#include <QPushButton>
#include <releaseinfo.h>
#include <appconfig.h>

WelcomePage::WelcomePage(QWidget *parent)
  : AShirtDialog{parent}
  , m_sideGrad{QLinearGradient(0,0, m_side_w, m_side_h)}

{
    m_sideGrad.setColorAt(0, QColor(0x2E, 0x33, 0x37));
    m_sideGrad.setColorAt(1, QColor(0x1D, 0x20, 0x24));
    setFixedSize(780, 468);
    QString buttonStyle = QStringLiteral("QPushButton:enabled{\
    background-color: rgba(93, 172, 232, 255);\
    font:;color:\"black\";}");

    auto f = font();

    auto _lblTitleLabel = new QLabel(this);
    f.setPointSize(m_titleFontSize);
    f.setBold(true);
    _lblTitleLabel->setFont(f);
    _lblTitleLabel->setText(tr("Welcome!"));

    auto _lblSubtitle = new QLabel(this);
    f.setPointSize(m_subTitleFontSize);
    f.setBold(false);
    _lblSubtitle->setFont(f);
    _lblSubtitle->setText(tr("Click on the Ashirt tray icon to begin"));

    auto topLayout = new QVBoxLayout();
    topLayout->addWidget(_lblTitleLabel);
    topLayout->addWidget(_lblSubtitle);
    auto tGrid = new QGridLayout();
    tGrid->setContentsMargins(215, 20, 20, 20);
    tGrid->addLayout(topLayout, 0, 0, 6, 5);
    tGrid->setVerticalSpacing(15);

    auto startSetup = new QPushButton(this);
    startSetup->setStyleSheet(buttonStyle);
    startSetup->setText(tr("Start Setup"));
    connect(startSetup, &QPushButton::clicked, this, [this]{
        close();
        Q_EMIT requestSetupWizard();
    });

    auto _lblTopLabel = new QLabel(this);
    f.setPointSize(m_h3FontSize);
    f.setBold(true);
    _lblTopLabel->setFont(f);
    _lblTopLabel->setText(tr("New Users"));

    auto _lblTopBody = new QLabel(this);
    f.setPointSize(m_bodyFontSize);
    f.setBold(false);
    _lblTopBody->setFont(f);
    _lblTopBody->setText(tr("On your first launch, you must setup an appropriate\nconfiguration with our setup wizard."));
    auto line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine);

    tGrid->addWidget(_lblTopLabel, 7, 0, 2, 5, Qt::AlignLeft | Qt::AlignTop);
    tGrid->addWidget(_lblTopBody, 8, 0, 4, 4, Qt::AlignLeft | Qt::AlignBottom);
    tGrid->addWidget(startSetup, 8, 5, 6, 1, Qt::AlignVCenter);
    tGrid->addWidget(line ,12 , 0, 1, 6, Qt::AlignVCenter);

    auto _lblMidLabel = new QLabel(this);
    f.setPointSize(m_h3FontSize);
    f.setBold(true);
    _lblMidLabel->setFont(f);
    _lblMidLabel->setText(tr("Explore the Server"));

    auto _lblMidBody = new QLabel(this);
    f.setPointSize(m_bodyFontSize);
    f.setBold(false);
    _lblMidBody->setFont(f);
    _lblMidBody->setText(tr("This is our frontend web toolkit that allows ease of use with\ntesting operations and viewing evience."));
    auto viewServer = new QPushButton(this);
    viewServer->setStyleSheet(buttonStyle);
    connect(viewServer, &QPushButton::clicked, this, [this] {
      QDesktopServices::openUrl(QUrl(QStringLiteral("https://github.com/ashirt-ops/ashirt-server"), QUrl::TolerantMode));
    });
    viewServer->setText(tr("Download"));

    auto line2 = new QFrame(this);
    line2->setFrameStyle(QFrame::HLine);

    tGrid->addWidget(_lblMidLabel, 13, 0, 2, 5, Qt::AlignLeft | Qt::AlignTop);
    tGrid->addWidget(_lblMidBody, 14, 0, 4, 4, Qt::AlignLeft | Qt::AlignBottom);
    tGrid->addWidget(viewServer, 14, 5, 6, 1, Qt::AlignVCenter);
    tGrid->addWidget(line2 ,18 , 0, 1, 6, Qt::AlignVCenter);

    auto _lblBottomLabel = new QLabel(this);
    f.setPointSize(m_h3FontSize);
    f.setBold(true);
    _lblBottomLabel->setFont(f);
    _lblBottomLabel->setText(tr("Quick Docs"));

    auto _lblBottomBody = new QLabel(this);
    f.setPointSize(m_bodyFontSize);
    f.setBold(false);
    _lblBottomBody->setFont(f);
    _lblBottomBody->setText(tr("For more information about ashirt, support and access to\ndocumentation or common user questions."));
    auto learnMore = new QPushButton(this);
    learnMore->setStyleSheet(buttonStyle);
    connect(learnMore, &QPushButton::clicked, this, [this] {
      QDesktopServices::openUrl(QUrl(QStringLiteral("https://ashirt.io"), QUrl::TolerantMode));
    });
    learnMore->setText(tr("Learn More"));

    tGrid->addWidget(_lblBottomLabel, 19, 0, 2, 5, Qt::AlignLeft | Qt::AlignTop);
    tGrid->addWidget(_lblBottomBody, 20, 0, 4, 4, Qt::AlignLeft | Qt::AlignBottom);
    tGrid->addWidget(learnMore, 20, 5, 6, 1, Qt::AlignVCenter);

    auto hide = new QCheckBox("Do Not show when starting", this);
    hide->setChecked(AppConfig::value(CONFIG::SHOW_WELCOME_SCREEN) != "true");
    connect(hide,&QCheckBox::stateChanged, this, [hide, this] (int newState) {
      if(newState != Qt::Checked)
        AppConfig::setValue(CONFIG::SHOW_WELCOME_SCREEN, "true");
      else
        AppConfig::setValue(CONFIG::SHOW_WELCOME_SCREEN, "false");
    });
    tGrid->addWidget(hide, 25, 0, 2, 6);

    setLayout(tGrid);
}

void WelcomePage::paintEvent(QPaintEvent *)
{
    static QFont titleFont = QFont("Helvetica Neue", m_sideLargeFont, QFont::ExtraBold, false);
    titleFont.setStyleHint(QFont::Helvetica, QFont::StyleStrategy(QFont::PreferQuality | QFont::PreferAntialias));

    static QFont subFont = QFont("Helvetica Neue", m_sideSmallFont, QFont::Light, true);
    subFont.setStyleHint(QFont::Helvetica, QFont::StyleStrategy(QFont::PreferQuality | QFont::PreferAntialias));

    static QFont versionFont = QFont("Helvetica Neue", m_sideSmallFont, QFont::ExtraLight, false);
    versionFont.setStyleHint(QFont::Helvetica, QFont::StyleStrategy(QFont::PreferQuality | QFont::PreferAntialias));

#ifdef Q_OS_MAC
    subFont.setWeight(QFont::Normal);
    versionFont.setWeight(QFont::Light);
#endif

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::TextAntialiasing, true);
    painter.fillRect(0,0, 200, height(), m_sideGrad);
    painter.drawPixmap(QRect(50, 135, 100, 100), QStringLiteral(":/icons/shirt-light.svg"));
    painter.setPen(QColor(0xF5,0xFB,0xFF));
    painter.setFont(titleFont);
    painter.drawText(QRect(0, 227, 200, 251), QStringLiteral("ASHIRT"), QTextOption(Qt::AlignHCenter));
    painter.setFont(subFont);
    painter.drawText(QRect(25, 275, 175, 350), QStringLiteral("Adversary Simulators High-Fedelity Intelligence and Reporting Toolkit"));
    painter.setFont(versionFont);
    painter.drawText(QRect(0, 445, 200, 455), QStringLiteral("Version %1").arg(ReleaseInfo::version), QTextOption(Qt::AlignHCenter));
    painter.end();
}
