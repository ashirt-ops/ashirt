#pragma once

#include "wizardpage.h"
#include <QObject>

class QLabel;
class FinishedPage : public WizardPage
{
  Q_OBJECT
 public:
  void initializePage() override;
  bool validatePage() override;
  FinishedPage(QWidget *parent = nullptr);
 private:
  QLabel * m_lblBody = nullptr;
};
