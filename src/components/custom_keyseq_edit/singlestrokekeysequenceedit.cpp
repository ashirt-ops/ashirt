#include "singlestrokekeysequenceedit.h"

SingleStrokeKeySequenceEdit::SingleStrokeKeySequenceEdit(QWidget* parent) : QKeySequenceEdit(parent){
  this->installEventFilter(this);
}

// Note: this may prevent editingFinished from firing
void SingleStrokeKeySequenceEdit::keyPressEvent(QKeyEvent * evt) {
  QKeySequenceEdit::keyPressEvent(evt);

  setKeySequence(keySequence()[0]);
  previousSequence = keySequence(); // update the saved sequence once one has been set
}

bool SingleStrokeKeySequenceEdit::eventFilter(QObject *object, QEvent *event) {
  if (event->type() == QEvent::FocusIn) {
    // save + remove the current sequence to provide the user with a prompt
    previousSequence = keySequence();
    clear();
  }
  if (event->type() == QEvent::FocusOut) {
    // restore the current/saved sequence
    setKeySequence(previousSequence);
    emit keySequenceChanged(previousSequence);
  }
  return QKeySequenceEdit::eventFilter(object, event);
}
