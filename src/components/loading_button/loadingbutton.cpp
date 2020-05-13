// Copyright 2020, Verizon Media
// Licensed under the terms of GPLv3. See LICENSE file in project root for terms.

#include "loadingbutton.h"

#include <QResizeEvent>

LoadingButton::LoadingButton(QWidget* parent, QPushButton* model)
    : LoadingButton("", parent, model) {}
LoadingButton::LoadingButton(const QString& text, QWidget* parent, QPushButton* model)
    : QPushButton(text, parent) {
  if (model == nullptr) {
    model = this;
  }
  this->setMinimumSize(model->width(), model->minimumHeight());
  this->resize(model->width(), model->height());
  this->showingLabel = true;

  loading = new QProgressIndicator(this);
  loading->setMinimumSize(this->minimumSize());
  loading->resize(this->width(), this->height());
}

LoadingButton::~LoadingButton() { delete loading; }

void LoadingButton::startAnimation() {
  label = this->text();
  showLabel(false);
}

void LoadingButton::stopAnimation() { showLabel(true); }

void LoadingButton::showLabel(bool show) {
  this->showingLabel = show;
  if (show) {
    loading->stopAnimation();
    this->setText(this->label);
  }
  else {
    this->setText("");
    loading->startAnimation();
  }
}

void LoadingButton::resizeEvent(QResizeEvent* evt) {
  QPushButton::resizeEvent(evt);
  loading->resize(evt->size());
}
