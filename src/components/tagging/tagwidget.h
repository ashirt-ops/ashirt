#ifndef TAGWIDGET_H
#define TAGWIDGET_H

#include <QImage>
#include <QLabel>
#include <QWidget>
#include <QRandomGenerator>
#include <unordered_map>

#include "dtos/tag.h"

static std::unordered_map<QString, QColor> colorMap{
    // matches colors defined on front end
    {"blue",           QColor(0x0E5A8A)},
    {"yellow",         QColor(0xA67908)},
    {"green",          QColor(0x0A6640)},
    {"indigo",         QColor(0x5642A6)},
    {"orange",         QColor(0xA66321)},
    {"pink",           QColor(0xA82255)},
    {"red",            QColor(0xA82A2A)},
    {"teal",           QColor(0x008075)},
    {"vermilion",      QColor(0x9E2B0E)},
    {"violet",         QColor(0x5C255C)},
    {"lightBlue",      QColor(0x48AFF0)},
    {"lightYellow",    QColor(0xFFC940)},
    {"lightGreen",     QColor(0x3DCC91)},
    {"lightIndigo",    QColor(0xAD99FF)},
    {"lightOrange",    QColor(0xFFB366)},
    {"lightPink",      QColor(0xFF66A1)},
    {"lightRed",       QColor(0xFF7373)},
    {"lightTeal",      QColor(0x2EE6D6)},
    {"lightVermilion", QColor(0xFF6E4A)},
    {"lightViolet",    QColor(0xC274C2)},
};

static std::vector<QString> allColorsNames = []()->std::vector<QString>{
  std::vector<QString> colors;
  for (auto kv : colorMap) {
    colors.push_back(kv.first);
  }
  return colors;
}();

class TagWidget : public QLabel {
  Q_OBJECT
 public:
  explicit TagWidget(dto::Tag tag, bool readonly, QWidget* parent = nullptr);
  ~TagWidget() = default;

 private:
  void buildUi();
  void wireUi();
  QImage mkNewImage();
  void setImage(QImage img);

 protected:
  void mouseReleaseEvent(QMouseEvent* ev) override;

 public:
  inline dto::Tag getTag(){return tag;};

  void setReadOnly(bool readonly);
  inline bool isReadOnly(){return readonly;}

  static QString randomColor() {
    // Note: this should match the frontend's color palette (naming)
    auto index = QRandomGenerator::global()->bounded(int(allColorsNames.size()));
    return allColorsNames.at(index);
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

  const QString removeSymbol = "\u2717";

  QRect labelArea;
  QRect removeArea;
  int tagWidth;
  int tagHeight;
};

#endif  // TAGWIDGET_H
