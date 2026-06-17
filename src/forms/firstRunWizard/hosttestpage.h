#pragma once

#include "wizardpage.h"
#include "helpers/netman.h"
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
  void testResultsChanged(NetMan::TestResult result);
  void timerCheck();
  int f=0;
  QTimer *timer = nullptr;
  QNetworkReply *currentTestReply = nullptr;
  NetMan::TestResult currentState = NetMan::TestResult::INPROGRESS;
};
