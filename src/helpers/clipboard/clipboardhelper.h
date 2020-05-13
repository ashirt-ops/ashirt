#ifndef CLIPBOARDHELPER_H
#define CLIPBOARDHELPER_H

#include <QApplication>
#include <QClipboard>
#include <QMimeData>
#include <QObject>
#include <QPixmap>

class ClipboardHelper : public QObject {
  Q_OBJECT
 public:
  explicit ClipboardHelper(QObject *parent = nullptr) = delete;

 public:
  static QString readPlaintext();
  static QPixmap readImage();

 signals:
};

#endif  // CLIPBOARDHELPER_H
