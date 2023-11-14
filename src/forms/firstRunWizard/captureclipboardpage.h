#pragma once

#include "wizardpage.h"
#include <QObject>

class SingleStrokeKeySequenceEdit;
class CaptureClipboardPage : public WizardPage
{
  Q_OBJECT
 public:
  bool validatePage() override;
  void initializePage() override;
  void cleanupPage() override;
  CaptureClipboardPage(QWidget *parent = nullptr);
 private:
  SingleStrokeKeySequenceEdit *captureClipboardKeySequence = nullptr;
};
