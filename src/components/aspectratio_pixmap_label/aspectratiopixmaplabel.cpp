// Copyright 22014-020, Phyatt, et al
// Licensed under the terms of CC BY-SA 3.0.
// Original Source: https://stackoverflow.com/a/22618496/4262552

#include "aspectratiopixmaplabel.h"

AspectRatioPixmapLabel::AspectRatioPixmapLabel(QWidget *parent) : QLabel(parent) {
  setMinimumSize(1, 1);
  setScaledContents(false);
}

void AspectRatioPixmapLabel::setPixmap(const QPixmap &p) {
  pix = p;
  QLabel::setPixmap(scaledPixmap());
}

int AspectRatioPixmapLabel::heightForWidth(int width) const {
  return pix.isNull() ? this->height() : ((qreal)pix.height() * width) / pix.width();
}

QSize AspectRatioPixmapLabel::sizeHint() const {
  // int w = this->width();
  // return QSize(w, heightForWidth(w));
  // js: Forcing a predetermined size as frequently the image is too small.
  return QSize(500, 500);
}

QPixmap AspectRatioPixmapLabel::scaledPixmap() const {
  return pix.scaled(this->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
}

void AspectRatioPixmapLabel::resizeEvent(QResizeEvent *) {
  if (!pix.isNull()) QLabel::setPixmap(scaledPixmap());
}
