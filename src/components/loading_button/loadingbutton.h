#pragma once

#include <QPushButton>

class QProgressIndicator;
class LoadingButton : public QPushButton {
  Q_OBJECT

 public:
  explicit LoadingButton(QWidget* parent = nullptr, QPushButton* model = nullptr);
  explicit LoadingButton(const QString& text, QWidget* parent = nullptr,
                         QPushButton* model = nullptr);

  ~LoadingButton() = default;

  void startAnimation();
  void stopAnimation();

 private:
  void showLabel(bool show);
  void init();

 protected:
  void resizeEvent(QResizeEvent* evt) override;

 private:
  QProgressIndicator* loading = nullptr;
  QString label;
  bool showingLabel;
};
