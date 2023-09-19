#pragma once

#include "wizardpage.h"
#include <QObject>

class QLineEdit;
class HostPathPage : public WizardPage
{
  Q_OBJECT
 public:
  bool validatePage() override;
  void initializePage() override;
  HostPathPage(QWidget *parent = nullptr);
 private:
  QLineEdit * hostPathLine = nullptr;
};
