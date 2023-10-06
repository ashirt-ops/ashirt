#include "singlestrokekeysequenceedit.h"
#include <QAbstractButton>
#include <QLineEdit>

SingleStrokeKeySequenceEdit::SingleStrokeKeySequenceEdit(QWidget* parent) : QKeySequenceEdit(parent){
  setClearButtonEnabled(true);
  setMaximumSequenceLength(1);
  findChild<QLineEdit*>()->setReadOnly(true);
  findChild<QLineEdit*>()->findChild<QAbstractButton*>()->setEnabled(true);
}
