// Copyright 2020, Verizon Media
// Licensed under the terms of GPLv3. See LICENSE file in project root for terms.

#ifndef CREDITS_H
#define CREDITS_H

#include <QDialog>
#include <QKeyEvent>

namespace Ui {
class Credits;
}

class Credits : public QDialog {
  Q_OBJECT

 private:
  void keyPressEvent(QKeyEvent *evt) override;

 public:
  explicit Credits(QWidget *parent = nullptr);
  ~Credits();

 private:
  Ui::Credits *ui;
};

#endif  // CREDITS_H
