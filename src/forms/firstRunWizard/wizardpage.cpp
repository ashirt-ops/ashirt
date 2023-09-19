#include "wizardpage.h"
#include <QPen>
#include <QPainter>
#include <QEvent>
#include "system_helpers.h"

WizardPage::WizardPage(int pageId, QWidget *parent)
    : QWizardPage{parent}
      , m_id(pageId)
      , m_sideGrad{QLinearGradient(0,0, m_side_w, m_side_h)}
{
  m_sideGrad.setColorAt(0, QColor(0x2E, 0x33, 0x37));
  m_sideGrad.setColorAt(1, QColor(0x1D, 0x20, 0x24));

  setPixmap(QWizard::WatermarkPixmap,  paintSideImage(m_id));
  setStyleSheet(isDarkMode() ? _darkStyle : _lightStyle);
}

void WizardPage::changeEvent(QEvent *event)
{
  if (event->type() == QEvent::PaletteChange) {
    setStyleSheet(isDarkMode() ? _darkStyle : _lightStyle);
    setPixmap(QWizard::WatermarkPixmap,  paintSideImage(m_id));
    event->accept();
    return;
  }
  event->ignore();
}

bool WizardPage::isDarkMode()
{
  return !SystemHelpers::isLightTheme();
}

QPixmap WizardPage::paintSideImage(int pageId)
{
  QPen inactiveBubbleOutline = QPen(QColor(0x2D, 0x31, 0x35), 1);
  QColor highlightedStep  = QColor(0x79, 0xC5, 0xFF);
  QString checkMark = QStringLiteral("✓");

  QPixmap sideImage(m_side_w + 20, m_side_h);
  QPainter painter;

  painter.begin(&sideImage);

  painter.setRenderHint(QPainter::Antialiasing, true);
  painter.setRenderHint(QPainter::TextAntialiasing, true);
  painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

  QString fontFamily = "Helvetica Neue";
#ifdef Q_OS_LINUX
  int sideFontSize = 14;
#elif defined Q_OS_WIN
  int sideFontSize = 16;
#elif defined Q_OS_MAC
  int sideFontSize = 22;
#endif

  static QFont currentItemFont = QFont(fontFamily, sideFontSize, QFont::ExtraBold);
  currentItemFont.setStyleHint(QFont::Helvetica, QFont::StyleStrategy(QFont::PreferQuality | QFont::PreferAntialias));

  static QFont nonCurrentItemFont = QFont(fontFamily, sideFontSize, QFont::Light);
  nonCurrentItemFont.setStyleHint(QFont::Helvetica, QFont::StyleStrategy(QFont::PreferQuality | QFont::PreferAntialias));

  static QFont bubbleTextFont = QFont(fontFamily, sideFontSize, QFont::Normal);
  bubbleTextFont.setStyleHint(QFont::Helvetica, QFont::StyleStrategy(QFont::PreferQuality | QFont::PreferAntialias));

  static QFont bubbleCheckFont = QFont(fontFamily, sideFontSize, QFont::Normal);
  bubbleCheckFont.setStyleHint(QFont::Helvetica, QFont::StyleStrategy(QFont::PreferQuality | QFont::NoSubpixelAntialias));

  static QFont stepsFont = QFont(fontFamily, smallFont.first, smallFont.second);
  stepsFont.setStyleHint(QFont::Helvetica, QFont::StyleStrategy(QFont::PreferQuality | QFont::PreferAntialias));

  QColor inactiveBubbleColor = QColor(0x2D, 0x32, 0x36);
  QString _textTemp;
  //Background
  painter.fillRect(0,0, sideImage.width() - 20, sideImage.height(), m_sideGrad);
  painter.fillRect(m_side_w, 0 , 20, m_side_h, QBrush(palette().base().color()));

  if(pageId == Page_Requiments) {
    painter.setFont(currentItemFont);
    painter.setPen(highlightedStep);
    painter.setBrush(highlightedStep);
  } else {
    painter.setFont(nonCurrentItemFont);
    painter.setPen(QPen(Qt::white));
    painter.setBrush(inactiveBubbleColor);
  }

  painter.drawText(QRect(6, 30, 165, 32), Qt::AlignRight | Qt::AlignVCenter, QStringLiteral("Requirements"));

  painter.setPen(inactiveBubbleOutline);
  painter.drawEllipse(184, 30,32,32);

  painter.setPen(Qt::white);
  if(pageId == Page_Requiments) {
    painter.setFont(bubbleTextFont);
    _textTemp = QStringLiteral("1");
  } else {
    painter.setFont(bubbleCheckFont);
    _textTemp = checkMark;
  }
  painter.drawText(QRect(184, 30,32,32), Qt::AlignCenter, _textTemp);

  if(pageId == Page_Evidence) {
    painter.setFont(currentItemFont);
    painter.setPen(highlightedStep);
    painter.setBrush(highlightedStep);
  } else {
    painter.setFont(nonCurrentItemFont);
    painter.setPen(Qt::white);
    painter.setBrush(inactiveBubbleColor);
  }

  painter.drawText(QRect(6,117,165,32), Qt::AlignRight | Qt::AlignVCenter, QStringLiteral("Evidence Path"));
  painter.setPen(inactiveBubbleOutline);
  painter.drawEllipse(184,117,32,32);

  painter.setPen(Qt::white);
  if(pageId < Page_HostPath) {
    painter.setFont(bubbleTextFont);
    _textTemp = QStringLiteral("2");
  } else {
    painter.setFont(bubbleCheckFont);
    _textTemp = checkMark;
  }
  painter.drawText(QRect(184, 117, 32,32), Qt::AlignCenter, _textTemp);

  if(pageId >= Page_HostPath && pageId < Page_CaptureArea) {
    painter.setFont(currentItemFont);
    painter.setPen(highlightedStep);
    painter.setBrush(highlightedStep);
  } else {
    painter.setFont(nonCurrentItemFont);
    painter.setPen(Qt::white);
    painter.setBrush(inactiveBubbleColor);
  }

  painter.drawText(QRect(6, 204, 165, 32), Qt::AlignRight | Qt::AlignVCenter, QStringLiteral("Server Settings"));
  painter.setPen(inactiveBubbleOutline);

  if(pageId == Page_HostPath) {
    painter.setBrush(highlightedStep);
    painter.drawPie(184,204,32,32, 90*16, -120*16);
    painter.setBrush(inactiveBubbleColor);
    painter.drawPie(184,204, 32,32, 90*16, 240*16);
    painter.setPen(Qt::white);
    painter.setFont(stepsFont);
    painter.drawText(QRect(0,229,200,16), Qt::AlignHCenter | Qt::AlignVCenter, QStringLiteral("Step 1 of 3"));
  } else if(pageId == Page_Api) {
    painter.setBrush(highlightedStep);
    painter.drawPie(184,204,32,32, 90*16, -240*16);
    painter.setBrush(inactiveBubbleColor);
    painter.drawPie(184,204,32,32, 90*16, 120*16);
    painter.setPen(Qt::white);
    painter.setFont(stepsFont);
    painter.drawText(QRect(0,229,200,16), Qt::AlignHCenter | Qt::AlignVCenter, QStringLiteral("Step 2 of 3"));
  } else if(pageId == Page_HostTest) {
    painter.drawEllipse(184,204,32,32);
    painter.setPen(Qt::white);
    painter.setFont(stepsFont);
    painter.drawText(QRect(0,229,200,16), Qt::AlignHCenter | Qt::AlignVCenter, QStringLiteral("Step 3 of 3"));
  } else {
    painter.drawEllipse(184,204,32,32);
  }

  painter.setPen(Qt::white);
  if(pageId < Page_CaptureArea) {
    painter.setFont(bubbleTextFont);
    _textTemp = QStringLiteral("3");
  } else {
    painter.setFont(bubbleCheckFont);
    _textTemp = checkMark;
  }
  painter.drawText(QRect(184,204,32,32), Qt::AlignCenter, _textTemp);

  if(pageId >= Page_CaptureArea && pageId < Page_Ops) {
    painter.setFont(currentItemFont);
    painter.setPen(highlightedStep);
    painter.setBrush(highlightedStep);
  } else {
    painter.setFont(nonCurrentItemFont);
    painter.setPen(Qt::white);
    painter.setBrush(inactiveBubbleColor);
  }

  painter.drawText(QRect(6, 291, 165, 32), Qt::AlignRight | Qt::AlignVCenter, QStringLiteral("Input Settings"));
  painter.setPen(inactiveBubbleOutline);
  painter.drawEllipse(184,291,32,32);

  if(pageId == Page_CaptureArea) {
    painter.setBrush(highlightedStep);
    painter.drawPie(184,291,32,32, 90*16, -120*16);
    painter.setBrush(inactiveBubbleColor);
    painter.drawPie(184,291, 32,32, 90*16, 240*16);
    painter.setPen(Qt::white);
    painter.setFont(stepsFont);
    painter.drawText(QRect(0,316,200,16), Qt::AlignHCenter | Qt::AlignVCenter, QStringLiteral("Step 1 of 3"));
  } else if(pageId == Page_CaptureWindow) {
    painter.setBrush(highlightedStep);
    painter.drawPie(184,291,32,32, 90*16, -240*16);
    painter.setBrush(inactiveBubbleColor);
    painter.drawPie(184,291,32,32, 90*16, 120*16);
    painter.setPen(Qt::white);
    painter.setFont(stepsFont);
    painter.drawText(QRect(0,316,200,16), Qt::AlignHCenter | Qt::AlignVCenter, QStringLiteral("Step 2 of 3"));
  } else if(pageId == Page_CaptureClipboard) {
    painter.drawEllipse(184,291,32,32);
    painter.setPen(Qt::white);
    painter.setFont(stepsFont);
    painter.drawText(QRect(0,316,200,16), Qt::AlignHCenter | Qt::AlignVCenter, QStringLiteral("Step 3 of 3"));
  } else {
    painter.drawEllipse(184,291,32,32);
  }


  painter.setPen(Qt::white);
  if(pageId < Page_Ops) {
    painter.setFont(bubbleTextFont);
    _textTemp = QStringLiteral("4");
  } else {
    painter.setFont(bubbleCheckFont);
    _textTemp = checkMark;
  }
  painter.drawText(QRect(184,291,32,32), Qt::AlignCenter, _textTemp);

  if(pageId >= Page_Ops && pageId < Page_Finished) {
    painter.setFont(currentItemFont);
    painter.setPen(highlightedStep);
    painter.setBrush(highlightedStep);
  } else {
    painter.setFont(nonCurrentItemFont);
    painter.setPen(Qt::white);
    painter.setBrush(inactiveBubbleColor);
  }

  painter.drawText(QRect(6, 378, 165, 32),Qt::AlignRight | Qt::AlignVCenter, QStringLiteral("Operation Name"));
  painter.setPen(inactiveBubbleOutline);
  painter.drawEllipse(184,378,32,32);

  painter.setPen(Qt::white);
  if(pageId != Page_Finished) {
    painter.setFont(bubbleTextFont);
    _textTemp = QStringLiteral("5");
  } else {
    painter.setFont(bubbleCheckFont);
    _textTemp = checkMark;
  }
  painter.drawText(QRect(184,378,32,32),Qt::AlignCenter, _textTemp);
  painter.end();

  return sideImage;
}
