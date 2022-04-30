// Copyright 2020, Verizon Media
// Licensed under the terms of MIT. See LICENSE file in project root for terms.

#pragma once

#include <QPushButton>

#include "components/loading/qprogressindicator.h"

class LoadingButton : public QPushButton {
  Q_OBJECT

 public:
  explicit LoadingButton(QWidget* parent = nullptr, QPushButton* model = nullptr);
  explicit LoadingButton(const QString& text, QWidget* parent = nullptr,
                         QPushButton* model = nullptr);

  ~LoadingButton();

  void startAnimation();
  void stopAnimation();

 private:
  void showLabel(bool show);
  void init();

 protected:
  void resizeEvent(QResizeEvent* evt) override;

 private:
  QProgressIndicator* loading;

  QString label;
  bool showingLabel;
};
