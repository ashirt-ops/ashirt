#include "singlestrokekeysequenceedit.h"

SingleStrokeKeySequenceEdit::SingleStrokeKeySequenceEdit(QWidget* parent) : QKeySequenceEdit(parent){

}

// Note: this may prevent editingFinished from firing
void SingleStrokeKeySequenceEdit::keyPressEvent(QKeyEvent * evt) {
  QKeySequenceEdit::keyPressEvent(evt);

  setKeySequence(keySequence()[0]);
}
