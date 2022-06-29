#pragma once

#include <QComboBox>
class UIHelpers {
 public:
  ///setComboBoxValue sets a comboBoxes currentIndex based on a "data" value
  ///Unlike findData the index 0 is returned if the item is not found.
  static void setComboBoxValue(QComboBox *box, QString dataValue) {
    box->setCurrentIndex(std::max(0, box->findData(dataValue)));
  }
};
