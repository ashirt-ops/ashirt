// Copyright 2020, Verizon Media
// Licensed under the terms of GPLv3. See LICENSE file in project root for terms.

#ifndef SETTINGS_H
#define SETTINGS_H

#include <QAction>
#include <QCloseEvent>
#include <QDialog>
#include <QErrorMessage>
#include <QNetworkReply>

#include "components/loading_button/loadingbutton.h"
#include "forms/buttonboxform.h"
#include "hotkeymanager.h"

namespace Ui {
class Settings;
}

class Settings : public ButtonBoxForm {
  Q_OBJECT

 public:
  explicit Settings(HotkeyManager *hotkeyManager, QWidget *parent = nullptr);
  ~Settings();

 private:
  void wireUi();

 public slots:
  void onSaveClicked() override;
  void onCancelClicked() override;

  void showEvent(QShowEvent *evt) override;
  void closeEvent(QCloseEvent *event) override;

  void onTestConnectionClicked();
  void onTestRequestComplete();
  void onBrowseClicked();

 private:
  Ui::Settings *ui;
  QErrorMessage *couldNotSaveSettingsMsg;
  LoadingButton *testConnectionButton;
  QAction* closeWindowAction = nullptr;

  QNetworkReply *currentTestReply = nullptr;

  HotkeyManager *hotkeyManager;  // borrowed pointer
};

#endif  // SETTINGS_H
