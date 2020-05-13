#include "evidencepreview.h"

EvidencePreview::EvidencePreview(QWidget *parent) : QWidget(parent) {}

void EvidencePreview::saveEvidence() {
  // no-op expected for view-only evidence
}

void EvidencePreview::setReadonly(bool readonly) { this->readonly = readonly; }
