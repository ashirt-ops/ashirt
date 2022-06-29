#include "errorview.h"

#include <QLabel>
#include <QVBoxLayout>

ErrorView::ErrorView(QString errorText, QWidget* parent)
    : EvidencePreview(parent)
    , errorLabel(new QLabel(errorText, this))
{
  auto layout = new QVBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(errorLabel);
}
