#pragma once

#include "wizardpage.h"
#include <QObject>

class QLineEdit;
class ApiKeysPage : public WizardPage
{
  Q_OBJECT
 public:
  bool validatePage() override;
  void initializePage() override;
  ApiKeysPage(QWidget *parent = nullptr);
 private:
  QLineEdit * accessKeyLine = nullptr;
  QLineEdit * secretKeyLine = nullptr;
};
