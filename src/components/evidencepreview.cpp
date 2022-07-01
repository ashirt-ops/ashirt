#include "evidencepreview.h"

EvidencePreview::EvidencePreview(QWidget *parent) : QWidget(parent) {}

bool EvidencePreview::saveEvidence() {
  return true;
}

void EvidencePreview::setReadonly(bool readonly) { this->readonly = readonly; }
