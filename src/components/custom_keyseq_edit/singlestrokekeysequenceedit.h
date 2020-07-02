#ifndef SINGLESTROKEKEYSEQUENCEEDIT_H
#define SINGLESTROKEKEYSEQUENCEEDIT_H

#include <QObject>
#include <QKeySequenceEdit>
#include <QKeyEvent>

class SingleStrokeKeySequenceEdit : public QKeySequenceEdit
{
 public:
  SingleStrokeKeySequenceEdit(QWidget* parent=nullptr);

 protected:
  void keyPressEvent(QKeyEvent *evt) override;

};

#endif // SINGLESTROKEKEYSEQUENCEEDIT_H
