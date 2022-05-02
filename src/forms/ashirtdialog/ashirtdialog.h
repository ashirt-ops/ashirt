#pragma once

#include <QDialog>

/**
 * @brief The AShirtDialog class represents a base class used for all dialogs.
 */
class AShirtDialog : public QDialog {
  Q_OBJECT

 public:
  ///Create a AShirtDialog.
  AShirtDialog(QWidget* parent = nullptr, Qt::WindowFlags windowFlags = Qt::WindowFlags());

  /// show Overridden Show forces window to top
  void show();

  ///Commonly used windowFlags
  inline static const Qt::WindowFlags commonWindowFlags =
              Qt::WindowTitleHint
              | Qt::CustomizeWindowHint
              | Qt::WindowStaysOnTopHint
              | Qt::WindowMinMaxButtonsHint
              | Qt::WindowCloseButtonHint;
};
