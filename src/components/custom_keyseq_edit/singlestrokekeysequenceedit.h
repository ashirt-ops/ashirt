#pragma once

#include <QKeySequenceEdit>

class SingleStrokeKeySequenceEdit : public QKeySequenceEdit
{
 Q_OBJECT
 public:
  SingleStrokeKeySequenceEdit(QWidget* parent=nullptr);
};
