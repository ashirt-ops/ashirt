#include "clipboardhelper.h"

#include <iostream>

QString ClipboardHelper::readPlaintext() {
  const QClipboard *clipboard = QApplication::clipboard();
  const QMimeData *mimeData = clipboard->mimeData();
  QString data;
  if (mimeData->hasHtml()) {
    data = (mimeData->text());
  }
  else if (mimeData->hasText()) {
    data = mimeData->text();
  }
  else {
    data.clear();
  }
  return data;
}

QPixmap ClipboardHelper::readImage() {
  const QClipboard *clipboard = QApplication::clipboard();
  const QMimeData *mimeData = clipboard->mimeData();
  QPixmap data;
  if (mimeData->hasImage()) {
    data = qvariant_cast<QPixmap>(mimeData->imageData());
  }
  return data;
}

void ClipboardHelper::setText(QString text) {
  auto clipboard = QApplication::clipboard();
  clipboard->setText(text);
}
