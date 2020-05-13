// Copyright 2020, Verizon Media
// Licensed under the terms of GPLv3. See LICENSE file in project root for terms.

#ifndef BUTTONBOXFORM_H
#define BUTTONBOXFORM_H

#include <QDialog>
#include <QDialogButtonBox>
#include <QPushButton>

class ButtonBoxForm : public QDialog {
  Q_OBJECT

 public:
  explicit ButtonBoxForm(QWidget* parent = nullptr);
  ~ButtonBoxForm() = default;
  void setButtonBox(QDialogButtonBox* buttonBox);
  virtual void onOkClicked();
  virtual void onSaveClicked();
  virtual void onApplyClicked();
  virtual void onCloseClicked();
  virtual void onCancelClicked();

 public slots:
  void routeButtonPress(QAbstractButton* btn);

 private:
  QDialogButtonBox* buttonBox = nullptr;
};

#endif  // BUTTONBOXFORM_H
