#pragma once

#include "wizardpage.h"
#include <QObject>

class QLineEdit;
class SingleStrokeKeySequenceEdit;
class CaptureAreaPage : public WizardPage
{
  Q_OBJECT
 public:
  bool validatePage() override;
  void initializePage() override;
  void cleanupPage() override;
  CaptureAreaPage(QWidget *parent = nullptr);
 private:
  QLineEdit *captureAreaLine = nullptr;
  SingleStrokeKeySequenceEdit *captureAreaKeySequenceEdit = nullptr;
};
