#pragma once

#include "wizardpage.h"
#include <QObject>

class QLineEdit;
class SingleStrokeKeySequenceEdit;
class CaptureWindowPage : public WizardPage
{
  Q_OBJECT
 public:
  bool validatePage() override;
  void initializePage() override;
  void cleanupPage() override;
  CaptureWindowPage(QWidget *parent = nullptr);
 private:
  QLineEdit *captureWindowLine = nullptr;
  SingleStrokeKeySequenceEdit *captureWindowKeySequence = nullptr;
};
