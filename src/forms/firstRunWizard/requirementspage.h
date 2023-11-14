#pragma once

#include "wizardpage.h"
#include <QObject>

class RequirementsPage : public WizardPage
{
  Q_OBJECT
 public:
  RequirementsPage(QWidget *parent = nullptr);
};
