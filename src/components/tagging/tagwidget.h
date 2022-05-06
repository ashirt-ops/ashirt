#pragma once

#include <QImage>
#include <QLabel>
#include <QWidget>
#include <QRandomGenerator>
#include <unordered_map>

#include "dtos/tag.h"

class TagWidget : public QLabel {
  Q_OBJECT
 public:
  explicit TagWidget(dto::Tag tag, bool readonly, QWidget* parent = nullptr);
  ~TagWidget() = default;

 private:
  void buildTag();
  //void setImage(QImage img);

 protected:
  void mouseReleaseEvent(QMouseEvent* ev) override;

 public:
  inline dto::Tag getTag(){return tag;};

  void setReadOnly(bool readonly);
  inline bool isReadOnly(){return readonly;}

  static QString randomColor() {
    // Note: this should match the frontend's color palette (naming)
    auto index = QRandomGenerator::global()->bounded(int(allColorNames.size()));
    return allColorNames.at(index);
  }

  static QColor fontColorForBgColor(QColor bg) {
    long yiq = ((bg.red() * 299) + (bg.green() * 587) + (bg.blue() * 114)) / 1000;
    return (yiq < 128 ? Qt::white : Qt::black);
  }

 signals:
  void removePressed();
  void labelPressed();

 private:
  bool readonly = false;
  dto::Tag tag;

  const QString removeSymbol = QString::fromUtf8("\u2718");

  QRectF labelArea;
  QRectF removeArea;
  int tagWidth;
  int tagHeight;
  inline static const QMap<QString, QColor> colorMap{
      // matches colors defined on front end
      {QStringLiteral("blue"),           QColor(0x0E5A8A)},
      {QStringLiteral("yellow"),         QColor(0xA67908)},
      {QStringLiteral("green"),          QColor(0x0A6640)},
      {QStringLiteral("indigo"),         QColor(0x5642A6)},
      {QStringLiteral("orange"),         QColor(0xA66321)},
      {QStringLiteral("pink"),           QColor(0xA82255)},
      {QStringLiteral("red"),            QColor(0xA82A2A)},
      {QStringLiteral("teal"),           QColor(0x008075)},
      {QStringLiteral("vermilion"),      QColor(0x9E2B0E)},
      {QStringLiteral("violet"),         QColor(0x5C255C)},
      {QStringLiteral("lightBlue"),      QColor(0x48AFF0)},
      {QStringLiteral("lightYellow"),    QColor(0xFFC940)},
      {QStringLiteral("lightGreen"),     QColor(0x3DCC91)},
      {QStringLiteral("lightIndigo"),    QColor(0xAD99FF)},
      {QStringLiteral("lightOrange"),    QColor(0xFFB366)},
      {QStringLiteral("lightPink"),      QColor(0xFF66A1)},
      {QStringLiteral("lightRed"),       QColor(0xFF7373)},
      {QStringLiteral("lightTeal"),      QColor(0x2EE6D6)},
      {QStringLiteral("lightVermilion"), QColor(0xFF6E4A)},
      {QStringLiteral("lightViolet"),    QColor(0xC274C2)},
  };
  inline static QStringList allColorNames = colorMap.keys();
  inline static int smBuffer = 6;
  inline static int lgBuffer = 12;
};
