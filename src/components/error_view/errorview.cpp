#include "errorview.h"
 #include <utility> 
ErrorView::ErrorView(QString errorText, QWidget* parent) : EvidencePreview(parent) {
  this->errorText = std::move(errorText);
  buildUi();
  wireUi();
}

ErrorView::~ErrorView() {
  delete errorLabel;
  delete gridLayout;
}

void ErrorView::buildUi() {
  gridLayout = new QGridLayout(this);
  gridLayout->setMargin(0);

  errorLabel = new QLabel(errorText, this);

  // Layout
  /*        0
       +------------+
    0  |            |
       | Err Label  |
       |            |
       +------------+
  */

  // row 0
  gridLayout->addWidget(errorLabel, 0, 0);
}

void ErrorView::wireUi() {}

// ---- Parent Overrides

void ErrorView::clearPreview() {}

void ErrorView::loadFromFile(QString filepath) { Q_UNUSED(filepath); }
