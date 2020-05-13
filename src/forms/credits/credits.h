// Copyright 2020, Verizon Media
// Licensed under the terms of GPLv3. See LICENSE file in project root for terms.

#ifndef CREDITS_H
#define CREDITS_H

#include <QDialog>

namespace Ui {
class Credits;
}

class Credits : public QDialog {
  Q_OBJECT

 public:
  explicit Credits(QWidget *parent = nullptr);
  ~Credits();

 private:
  Ui::Credits *ui;
};

#endif  // CREDITS_H
