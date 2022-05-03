// Copyright 22014-020, Phyatt, et al
// Licensed under the terms of CC BY-SA 3.0.
// Original Source: https://stackoverflow.com/a/22618496/4262552

#pragma once

#include <QLabel>
#include <QPixmap>
#include <QResizeEvent>

class AspectRatioPixmapLabel : public QLabel {
  Q_OBJECT
 public:
  explicit AspectRatioPixmapLabel(QWidget *parent = nullptr);
  virtual int heightForWidth(int width) const;
  virtual QSize sizeHint() const;
  QPixmap scaledPixmap() const;
 public slots:
  void setPixmap(const QPixmap &p);
 protected:
  void resizeEvent(QResizeEvent *);

 private:
  QPixmap pix;
};
