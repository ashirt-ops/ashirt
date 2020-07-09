#include "tagwidget.h"

#include <QPainter>
#include <QMouseEvent>
#include <iostream>

TagWidget::TagWidget(dto::Tag tag, bool readonly, QWidget *parent) : QLabel(parent) {
  this->tag = tag;
  this->readonly = readonly;
  buildUi();
}

void TagWidget::buildUi() {
  setImage(mkNewImage());
}

void TagWidget::wireUi(){

}

void TagWidget::setReadOnly(bool readonly) {
  this->readonly = readonly;
  setImage(mkNewImage());
}

void TagWidget::mouseReleaseEvent(QMouseEvent* evt) {
  const int x = evt->x();
  const int y = evt->y();

  if (removeArea.contains(x, y)) {
    emit removePressed();
  }
  else if (labelArea.contains(x, y)) {
    emit labelPressed();
  }
}

void TagWidget::setImage(QImage img) {
  setPixmap(QPixmap::fromImage(img));
  tagWidth = img.width();
  tagHeight = img.height();
}

QImage TagWidget::mkNewImage() {
  QFont labelFont;
  labelFont.setFamily("Helvetica");
  labelFont.setPixelSize(14);

  // Calculate the positions of everything
  QFontMetrics metric(labelFont);
  QSize labelSize = metric.size(Qt::TextSingleLine, tag.name);
  QSize removeSize = metric.size(Qt::TextSingleLine, removeSymbol);

  int labelWidth = labelSize.width();
  int innerTagHeight = std::max(labelSize.height(), removeSize.height()); //tag height without the buffer

  int tagHeightBuffer = 12; // space around the top/bottom (real size is half as much)
  int tagWidthBuffer = 12; // space around the outer left/right edges (real size is half as much)
  int innerBuffer = 6;  // space between each segment

  int labelLeftOffset = tagWidthBuffer/2;
  int labelTopOffset = ((innerTagHeight - labelSize.height())/2) + (tagHeightBuffer/2);

  int removeLeftOffset = labelLeftOffset + labelWidth + innerBuffer;
  int removeTopOffset = ((innerTagHeight - removeSize.height())/2) + (tagHeightBuffer/2);

  int fullTagWidth = labelWidth + tagWidthBuffer;
  int fullTagHeight = innerTagHeight + tagHeightBuffer;

  // set bounds for mouse release event
  labelArea = QRect(0, 0, removeLeftOffset, fullTagHeight);
  removeArea = QRect(-1, -1, 0, 0); // set to dummy value in case we don't have a remove area

  if (!readonly) {
    fullTagWidth += removeSize.width() + innerBuffer;
    removeArea = QRect(removeLeftOffset, 0, fullTagWidth - removeLeftOffset, fullTagHeight);
  }

  // prep the image
  QImage img(fullTagWidth, fullTagHeight, QImage::Format_ARGB32_Premultiplied);
  img.fill(qRgba(0,0,0,0)); // not sure if we need this

  QColor bgColor = colorMap[tag.colorName];
  QPainter painter;
  painter.begin(&img);

  // draw container
  painter.setBrush(bgColor);
  painter.setPen(Qt::NoPen);
  painter.drawRoundedRect(QRect(0, 0, img.width()-1, img.height()-1), 4, 4);

  // set up font drawing
  auto fontColor = fontColorForBgColor(bgColor);
  painter.setFont(labelFont);
  painter.setPen(fontColor);

  // draw label
  painter.drawText(QRect(QPoint(labelLeftOffset, labelTopOffset), labelSize), Qt::AlignCenter, tag.name);

  if(!readonly) {
    // draw remove
    labelFont.setBold(false);
    painter.drawText(QRect(QPoint(removeLeftOffset, removeTopOffset), removeSize), Qt::AlignCenter, removeSymbol);
  }
  painter.end();

  return img;
}
