#include "loadingbutton.h"

#include <QResizeEvent>

#include "components/loading/qprogressindicator.h"

LoadingButton::LoadingButton(QWidget* parent, QPushButton* model)
  : LoadingButton(QString(), parent, model) {}

LoadingButton::LoadingButton(const QString& text, QWidget* parent, QPushButton* model)
  : QPushButton(text, parent)
  , loading(new QProgressIndicator(this))
  , showingLabel(true)
{
  if (model == nullptr) {
    model = this;
  }
  setMinimumSize(model->width(), model->minimumHeight());
  resize(model->width(), model->height());

  loading->setMinimumSize(this->minimumSize());
  loading->resize(this->width(), this->height());
}

void LoadingButton::startAnimation() {
  label = text();
  showLabel(false);
}

void LoadingButton::stopAnimation() { showLabel(true); }

void LoadingButton::showLabel(bool show) {
  showingLabel = show;
  if (show) {
    loading->stopAnimation();
    setText(label);
  }
  else {
    setText(QString());
    loading->startAnimation();
  }
}

void LoadingButton::resizeEvent(QResizeEvent* evt) {
  QPushButton::resizeEvent(evt);
  loading->resize(evt->size());
}
