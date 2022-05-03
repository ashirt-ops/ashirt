#pragma once

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

    QString combo = QKeySequence(ke->key()).toString();
    if (ke->modifiers() & Qt::ShiftModifier) combo.prepend(QStringLiteral("Shift+"));
    if (ke->modifiers() & Qt::ControlModifier) combo.prepend(QStringLiteral("Ctrl+"));
    if (ke->modifiers() & Qt::AltModifier) combo.prepend(QStringLiteral("Alt+"));
    if (ke->modifiers() & Qt::MetaModifier) combo.prepend(QStringLiteral("Meta+"));

    return (QKeySequence(combo) == keyCombo);
  }

 protected:
  bool eventFilter(QObject *object, QEvent *event) {
    if (event->type() == QEvent::KeyPress) {
      QKeyEvent *ke = static_cast<QKeyEvent *>(event);

      if (matchesKey(ke, QKeySequence(Qt::CTRL | Qt::Key_Space))) {
        Q_EMIT completePressed();
        return true;
      }

      if (ke->key() == Qt::Key_Up) {
        Q_EMIT upPressed();
        return true;
      }
      if (ke->key() == Qt::Key_Down) {
        Q_EMIT downPressed();
        return true;
      }
    }
    else if (event->type() == QEvent::MouseButtonRelease) {
      auto mouseEvt = static_cast<QMouseEvent *>(event);
      if (mouseEvt->button() == Qt::LeftButton) {
        Q_EMIT leftMouseClickPressed();
        return true;
      }
    }
    return QObject::eventFilter(object, event);
  }
};
