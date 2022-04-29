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
  buildTag();
}

void TagWidget::wireUi(){

}

void TagWidget::setReadOnly(bool readonly) {
  this->readonly = readonly;
  buildTag();
}

void TagWidget::mouseReleaseEvent(QMouseEvent* evt) {
  const int x = evt->position().x();
  const int y = evt->position().y();

  if (removeArea.contains(x, y)) {
    emit removePressed();
  }
  else if (labelArea.contains(x, y)) {
    emit labelPressed();
  }
}

void TagWidget::buildTag() {
  QFont labelFont;
#ifdef Q_OS_MACOS
  labelFont = QFont("Arial", 14);
#elif defined(Q_OS_LINUX)
  labelFont = QFont("Liberation Sans", 12);
#else
  labelFont = QFont("Sans", 12);
#endif

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
  labelArea = QRectF(0, 0, removeLeftOffset, fullTagHeight);
  removeArea = QRectF(-1, -1, 0, 0); // set to dummy value in case we don't have a remove area

  if (!readonly) {
    fullTagWidth += removeSize.width() + innerBuffer;
    removeArea = QRectF(removeLeftOffset, 0, fullTagWidth - removeLeftOffset, fullTagHeight);
  }

  const qreal dpr = this->devicePixelRatio();

  // prep the image
  QPixmap pixmap = QPixmap(fullTagWidth * dpr, fullTagHeight * dpr);
  pixmap.setDevicePixelRatio(dpr);
  pixmap.fill(Qt::transparent);

  QColor bgColor = colorMap[tag.colorName];
  QPainter painter(&pixmap);
  // these actually are used and removing them makes the edges/text slightly less sharp
  painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);

  // draw container
  painter.setBrush(bgColor);
  painter.setPen(Qt::NoPen);
  painter.drawRoundedRect(QRectF(0, 0, (pixmap.width() / dpr)-1, (pixmap.height() / dpr)-1), 6, 6);

  // set up font drawing
  auto fontColor = fontColorForBgColor(bgColor);
  painter.setFont(labelFont);
  painter.setPen(fontColor);

  // draw label
  painter.drawText(QRectF(QPointF(labelLeftOffset, labelTopOffset), labelSize), Qt::AlignCenter, tag.name);

  // draw remove (if needed)
  if(!readonly) {
    painter.drawText(QRectF(QPointF(removeLeftOffset, removeTopOffset), removeSize), Qt::AlignCenter, removeSymbol);
  }
  painter.end();

  setPixmap(pixmap);
}
