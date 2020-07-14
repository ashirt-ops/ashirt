#ifndef UPDOWNKEYFILTER_H
#define UPDOWNKEYFILTER_H

#include <QEvent>
#include <QKeyEvent>
#include <QKeySequence>
#include <QWidget>
#include <iostream>

class TaggingLineEditEventFilter : public QObject {
  Q_OBJECT
 public:
  explicit TaggingLineEditEventFilter(QObject *parent = nullptr) : QObject(parent) {}

 signals:
  void upPressed();
  void downPressed();
  void completePressed();
  void leftMouseClickPressed();

 private:
  bool matchesKey(QKeyEvent *ke, QKeySequence keyCombo) {
    // with help from https://forum.qt.io/topic/73408/qt-reading-key-sequences-from-key-event/3

    QString modifier;
    QString key;

    if (ke->modifiers() & Qt::ShiftModifier) modifier += "Shift+";
    if (ke->modifiers() & Qt::ControlModifier) modifier += "Ctrl+";
    if (ke->modifiers() & Qt::AltModifier) modifier += "Alt+";
    if (ke->modifiers() & Qt::MetaModifier) modifier += "Meta+";

    key = QKeySequence(ke->key()).toString();

    QKeySequence ks(modifier + key);
    return ks[0] == keyCombo[0];
  }

 protected:
  bool eventFilter(QObject *object, QEvent *event) {
    if (event->type() == QEvent::KeyPress) {
      QKeyEvent *ke = static_cast<QKeyEvent *>(event);

      if (matchesKey(ke, QKeySequence(Qt::CTRL + Qt::Key_Space))) {
        emit completePressed();
        return true;
      }

      if (ke->key() == Qt::Key_Up) {
        emit upPressed();
        return true;
      }
      if (ke->key() == Qt::Key_Down) {
        emit downPressed();
        return true;
      }
    }
    else if (event->type() == QEvent::MouseButtonRelease) {
      auto mouseEvt = static_cast<QMouseEvent *>(event);
      if (mouseEvt->button() == Qt::LeftButton) {
        emit leftMouseClickPressed();
        return true;
      }
    }
    return QObject::eventFilter(object, event);
  }
};

#endif  // UPDOWNKEYFILTER_H
