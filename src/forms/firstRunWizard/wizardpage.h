#pragma once

#include <QWizardPage>
#include <QObject>

class WizardPage : public QWizardPage
{
  Q_OBJECT
 public:
  enum PAGE {
    Page_Requiments = 0,
    Page_Evidence,
    Page_HostPath,
    Page_Api,
    Page_HostTest,
    Page_CaptureArea,
    Page_CaptureWindow,
    Page_CaptureClipboard,
    Page_Ops,
    Page_Finished
  };
  WizardPage(int pageId = Page_Requiments, QWidget *parent = nullptr);
  int id() {return m_id;}
 protected:
  void changeEvent(QEvent *event);
  bool isDarkMode();
#ifdef Q_OS_MAC
  const QPair<int, QFont::Weight> titleFont = QPair<int, QFont::Weight>(28, QFont::Bold);
  const QPair<int, QFont::Weight> subTitleFont = QPair<int, QFont::Weight>(20, QFont::Normal);
  const QPair<int, QFont::Weight> bodyFont = QPair<int, QFont::Weight>(18, QFont::Normal);
  const QPair<int, QFont::Weight> smallFont = QPair<int, QFont::Weight>(12, QFont::Light);
#else
  const QPair<int, QFont::Weight> titleFont = QPair<int, QFont::Weight>(22, QFont::Bold);
  const QPair<int, QFont::Weight> subTitleFont = QPair<int, QFont::Weight>(14, QFont::Normal);
  const QPair<int, QFont::Weight> bodyFont = QPair<int, QFont::Weight>(12, QFont::Normal);
  const QPair<int, QFont::Weight> smallFont = QPair<int, QFont::Weight>(9, QFont::Normal);
#endif
 private:
  const int m_side_w = 200;
  const int m_side_h = 468;
  const int m_id;
  QLinearGradient m_sideGrad;
  QString _styleTemplate = QStringLiteral("QComboBox:editable,QLineEdit{background:rgba(%1,%2,%3,255);}");
  QString _lightStyle = _styleTemplate.arg(QStringLiteral("235"), QStringLiteral("235"), QStringLiteral("235"));
  QString _darkStyle = _styleTemplate.arg(QStringLiteral("21"), QStringLiteral("23"), QStringLiteral("26"));

  QPixmap paintSideImage(int pageId);
};
