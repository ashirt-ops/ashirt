#pragma once

#include <QObject>
#include <QKeySequenceEdit>
#include <QKeyEvent>

class SingleStrokeKeySequenceEdit : public QKeySequenceEdit
{
 public:
  SingleStrokeKeySequenceEdit(QWidget* parent=nullptr);

 protected:
  void keyPressEvent(QKeyEvent *evt) override;

  bool eventFilter(QObject *object, QEvent *event) override;

 private:
  QKeySequence previousSequence;
};
