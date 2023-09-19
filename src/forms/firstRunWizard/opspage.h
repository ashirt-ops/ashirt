#pragma once

#include "wizardpage.h"
#include <QObject>

#include "dtos/operation.h"

class QNetworkReply;
class QLabel;
class QComboBox;
class OpsPage : public WizardPage
{
  Q_OBJECT
 public:
  bool validatePage() override;
  void initializePage() override;
  OpsPage(QWidget *parent = nullptr);
 private:
  void operationsUpdated(bool success, const QList<dto::Operation> & operations);
  void createOpComplete();
  QLabel *responseLabel = nullptr;
  QComboBox *opsCombo = nullptr;
  QNetworkReply* createOpReply = nullptr;
  QMap<QString, QString> _OpsList;
  inline static const QRegularExpression invalidCharsRegex = QRegularExpression(QStringLiteral("[^A-Za-z0-9]+"));
  inline static const QRegularExpression startOrEndDash = QRegularExpression(QStringLiteral("^-|-$"));
  bool successful= false;
};
