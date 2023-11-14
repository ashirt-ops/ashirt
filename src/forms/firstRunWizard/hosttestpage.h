#pragma once

#include "wizardpage.h"
#include <QObject>

class QLabel;
class QNetworkReply;
class HostTestPage : public WizardPage
{
  Q_OBJECT
 public:
  HostTestPage(QWidget *parent = nullptr);
 protected:
  bool validatePage() override;
  void initializePage() override;
  void paintEvent(QPaintEvent*) override;
 private:
  void testResultsChanged(int result);
  void timerCheck();
  int f=0;
  QTimer *timer = nullptr;
  QNetworkReply *currentTestReply = nullptr;
  int currentState;
};
