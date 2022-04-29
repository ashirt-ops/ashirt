#pragma once

#include "ashirtdialog/ashirtdialog.h"

#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QNetworkReply>

#include "components/loading_button/loadingbutton.h"

class CreateOperation : public AShirtDialog {
  Q_OBJECT

 public:
  explicit CreateOperation(QWidget *parent = nullptr);
  ~CreateOperation();

 private:
  void buildUi();
  void wireUi();

  void submitButtonClicked();

 private slots:
  void onRequestComplete();


  QString makeSlugFromName(QString name);
//  void showEvent(QShowEvent *evt) override;

 private:

  QNetworkReply* createOpReply = nullptr;

  // ui elements
  QGridLayout* gridLayout = nullptr;
  LoadingButton* submitButton = nullptr;
  QLabel* _operationLabel = nullptr;
  QLabel* responseLabel = nullptr;
  QLineEdit* operationNameTextBox = nullptr;
};
