#ifndef SAVEEVIDENCERESPONSE_H
#define SAVEEVIDENCERESPONSE_H

#include <QString>

#include "models/evidence.h"

struct SaveEvidenceResponse {
  SaveEvidenceResponse(model::Evidence model) {
    this->model = model;
  }
  SaveEvidenceResponse(bool success, QString err, model::Evidence model)
      : SaveEvidenceResponse(model) {
    this->actionSucceeded = success;
    this->errorText = err;
  }
  bool actionSucceeded;
  QString errorText;
  model::Evidence model;
};

#endif  // SAVEEVIDENCERESPONSE_H
