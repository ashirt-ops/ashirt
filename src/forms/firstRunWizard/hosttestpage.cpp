#include "hosttestpage.h"

#include <QLabel>
#include <QMovie>
#include <QPaintEvent>
#include <QPainter>
#include <QVBoxLayout>
#include <QTimer>
#include <QNetworkReply>
#include <QCheckBox>

#include "helpers/netman.h"

HostTestPage::HostTestPage(QWidget *parent)
    : WizardPage{Page_HostTest, parent}
    , timer{new QTimer(this)}
{
  setCommitPage(true);
  timer->setSingleShot(false);

  connect(NetMan::get(), &NetMan::testStatusChanged, this, &HostTestPage::testResultsChanged);

  connect(timer, &QTimer::timeout, this, [this] {
    f = (f > 359) ? 0 : f+=1;
    update();
  });
  auto f = font();
  auto _lblTitleLabel = new QLabel(this);
  f.setPointSize(titleFont.first);
  f.setWeight(titleFont.second);
  _lblTitleLabel->setFont(f);
  _lblTitleLabel->setText(tr("Testing Server Connection"));

  //Hidden Check box to allow us to have a manditory key for going foward
  auto isReady = new QCheckBox(this);
  isReady->setVisible(false);
  registerField("host.accessable*", isReady);

  auto layout = new QVBoxLayout();
  layout->addWidget(_lblTitleLabel);
  layout->addSpacerItem(new QSpacerItem(0,300,QSizePolicy::Minimum, QSizePolicy::Expanding));
  setLayout(layout);
}

bool HostTestPage::validatePage()
{
  if(!field("host.accessable").toBool())
    return false;

  AppConfig::setValue(CONFIG::ACCESSKEY, field("host.accessKey").toString());
  AppConfig::setValue(CONFIG::SECRETKEY, field("host.secretKey").toString());
  AppConfig::setValue(CONFIG::APIURL, field("host.url").toString());
  return true;
}

void HostTestPage::initializePage()
{
  f=0;
  setField("host.accessable", false);
  connect(wizard(), &QWizard::currentIdChanged, this, &HostTestPage::timerCheck, Qt::UniqueConnection);
  NetMan::testConnection(field("host.url").toString(), field("host.accessKey").toString(), field("host.secretKey").toString());
}

void HostTestPage::paintEvent(QPaintEvent *event)
{
  WizardPage::paintEvent(event);
  QPixmap tmp(250,250);
  QPainter p(&tmp);

  p.fillRect(tmp.rect(), palette().base());
  p.setRenderHint(QPainter::Antialiasing, true);
  p.setRenderHint(QPainter::TextAntialiasing, true);
  p.setPen(QPen(palette().text().color(), 8));
  p.drawEllipse(25, 25, 200, 200);

  if ( currentState == NetMan::INPROGRESS) {
      p.translate(125, 125);
      p.rotate(f);
      p.translate(-125, -125);
      p.setBrush(QBrush( isDarkMode() ? QColor(0x6B,0x7F,0x8B) : QColor(0x5B,0x6C,0x76)));
      p.drawEllipse(113, 13, 24,24);
  }
  p.end();

  QRect drawArea (154,88,250,250);
  QRect errorArea (0, 351, 555, 48);

  static QFont errorFont = QFont("Helvetica Neue", 16, QFont::Normal);
  errorFont.setStyleStrategy(QFont::StyleStrategy(QFont::PreferQuality | QFont::PreferAntialias));

  static QFont statusFont = QFont("Helvetica Neue", 24, QFont::Normal);
  statusFont.setStyleStrategy(QFont::StyleStrategy(QFont::PreferQuality | QFont::PreferAntialias));

  static QFont checkFont = QFont("Helvetica Neue", 42, QFont::ExtraBold);
  checkFont.setStyleStrategy(QFont::StyleStrategy(QFont::PreferQuality | QFont::NoSubpixelAntialias));

  QPainter t (this);
  t.setRenderHint(QPainter::TextAntialiasing, true);
  t.setRenderHint(QPainter::Antialiasing, true);

  t.drawPixmap(drawArea, tmp);

  QTextOption textOptions;
  textOptions.setAlignment(Qt::AlignCenter);

  t.setFont(errorFont);
  t.drawText(errorArea, NetMan::lastTestError(), textOptions);

  t.translate(154, 88);
  QRect iconArea (91, 50, 72, 72);
  QRect labelArea (55,130,140,48);
  QRect testLabelArea(55,100,140,48);

  t.setPen(palette().text().color());
  switch (currentState) {
    case NetMan::INPROGRESS:
      t.setFont(statusFont);
      t.drawText(testLabelArea, "Testing", textOptions);
    break;
    case NetMan::SUCCESS:
      t.setFont(checkFont);
      t.drawText(iconArea, QStringLiteral("✓"), textOptions);
      t.setFont(statusFont);
      t.drawText(labelArea, "Success!", textOptions) ;
    break;
    default:
      t.setFont(checkFont);
      t.drawText(iconArea, QStringLiteral("X"), textOptions);
      t.setFont(statusFont);
      t.drawText(labelArea, "Failed", textOptions);
    break;
  };
  t.end();

}

void HostTestPage::testResultsChanged(int result)
{
  currentState = result;
  timerCheck();
  setField("host.accessable", (currentState == NetMan::SUCCESS));
  update();
}

void HostTestPage::timerCheck()
{
  if(wizard()->currentId() == id()) {
    f=0;
    if (currentState == NetMan::INPROGRESS)
      timer->start(33);
    else
      timer->stop();
  }
  else
    timer->stop();
}
