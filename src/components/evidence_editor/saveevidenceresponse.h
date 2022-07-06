#pragma once

#include "models/evidence.h"

struct SaveEvidenceResponse {
  SaveEvidenceResponse(model::Evidence model)
    : model(model) { }

  SaveEvidenceResponse(bool success, QString err, model::Evidence model)
    : model(model)
    , actionSucceeded(success)
    , errorText(err) { }

  bool actionSucceeded;
  QString errorText;
  model::Evidence model;
};
