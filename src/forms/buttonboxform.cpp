// Copyright 2020, Verizon Media
// Licensed under the terms of GPLv3. See LICENSE file in project root for terms.

#include "buttonboxform.h"

#include <iostream>

ButtonBoxForm::ButtonBoxForm(QWidget *parent) : QDialog(parent) {}

void ButtonBoxForm::setButtonBox(QDialogButtonBox *buttonBox) { this->buttonBox = buttonBox; }

void ButtonBoxForm::routeButtonPress(QAbstractButton *btn) {
  if (buttonBox == nullptr) {
    return;
  }
  if (buttonBox->button(QDialogButtonBox::Ok) == btn) {
    onOkClicked();
  }
  if (buttonBox->button(QDialogButtonBox::Save) == btn) {
    onSaveClicked();
  }
  else if (buttonBox->button(QDialogButtonBox::Apply) == btn) {
    onApplyClicked();
  }
  else if (buttonBox->button(QDialogButtonBox::Close) == btn) {
    onCloseClicked();
  }
  else if (buttonBox->button(QDialogButtonBox::Cancel) == btn) {
    onCancelClicked();
  }
}

// default functions that just ignore the result, for easier use.
void ButtonBoxForm::onOkClicked() {}
void ButtonBoxForm::onSaveClicked() {}
void ButtonBoxForm::onApplyClicked() {}
void ButtonBoxForm::onCloseClicked() {}
void ButtonBoxForm::onCancelClicked() {}
