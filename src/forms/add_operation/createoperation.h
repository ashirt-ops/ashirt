#pragma once

#include "ashirtdialog/ashirtdialog.h"

class QLabel;
class QLineEdit;
class QNetworkReply;
class LoadingButton;

class CreateOperation : public AShirtDialog {
  Q_OBJECT

 public:
  explicit CreateOperation(QWidget *parent = nullptr);
  ~CreateOperation();

 private:
  void submitButtonClicked();

 private slots:
  void onRequestComplete();
  QString makeSlugFromName(QString name);

 private:
  QNetworkReply* createOpReply = nullptr;
  // ui elements
  LoadingButton* submitButton = nullptr;
  QLabel* responseLabel = nullptr;
  QLineEdit* operationNameTextBox = nullptr;
};
