#pragma once

#include <QKeySequenceEdit>
#include <QKeyEvent>

class SingleStrokeKeySequenceEdit : public QKeySequenceEdit
{
 Q_OBJECT
 public:
  SingleStrokeKeySequenceEdit(QWidget* parent=nullptr);

 protected:
  void keyPressEvent(QKeyEvent *evt) override;

 private:
  QKeySequence previousSequence;
};
