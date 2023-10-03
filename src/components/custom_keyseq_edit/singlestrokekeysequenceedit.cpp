#include "singlestrokekeysequenceedit.h"

SingleStrokeKeySequenceEdit::SingleStrokeKeySequenceEdit(QWidget* parent) : QKeySequenceEdit(parent){
  setClearButtonEnabled(true);
}

// Note: this may prevent editingFinished from firing
void SingleStrokeKeySequenceEdit::keyPressEvent(QKeyEvent * evt) {
  QKeySequenceEdit::keyPressEvent(evt);

  setKeySequence(keySequence()[0]);
  previousSequence = keySequence(); // update the saved sequence once one has been set
  Q_EMIT keySequenceChanged(previousSequence);
}
