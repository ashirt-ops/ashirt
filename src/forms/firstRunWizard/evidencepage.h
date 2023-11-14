#pragma once

#include "wizardpage.h"
#include <QObject>

class QLineEdit;
class QLabel;
class EvidencePage : public WizardPage
{
  Q_OBJECT
 public:
  bool validatePage() override;
  void initializePage() override;
  EvidencePage(QWidget *parent = nullptr);
 private:
  QLineEdit * evidenceLine = nullptr;
  QLabel *errorLabel = nullptr;
};
