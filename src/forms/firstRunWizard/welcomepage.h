#pragma once
#include <QWidget>
#include "ashirtdialog/ashirtdialog.h"

class WelcomePage : public AShirtDialog
{
  Q_OBJECT
 public:
  explicit WelcomePage(QWidget *parent = nullptr);

 signals:
  void requestSetupWizard();
 protected:
  void paintEvent(QPaintEvent *);
 private:
  const int m_side_w = 200;
  const int m_side_h = 468;
  QLinearGradient m_sideGrad;
#ifdef Q_OS_MAC
  const int m_titleFontSize = 28;
  const int m_subTitleFontSize = 20;
  const int m_h3FontSize = 18;
  const int m_bodyFontSize = 14;
  const int m_sideLargeFont = 28;
  const int m_sideSmallFont = 12;

#else
  const int m_titleFontSize = 22;
  const int m_subTitleFontSize = 16;
  const int m_h3FontSize = 14;
  const int m_bodyFontSize = 10;
  const int m_sideLargeFont = 26;
  const int m_sideSmallFont = 10;
#endif
};
