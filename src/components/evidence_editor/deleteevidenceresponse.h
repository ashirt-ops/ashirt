#pragma once

#include "models/evidence.h"

struct DeleteEvidenceResponse {
  DeleteEvidenceResponse(model::Evidence model)
    : model(model){ }

  DeleteEvidenceResponse(bool fileDeleteSuccess, bool dbDeleteSuccess,
                                        QString err, model::Evidence model)
    : model( model)
    , fileDeleteSuccess(fileDeleteSuccess)
    , dbDeleteSuccess(dbDeleteSuccess)
    , errorText(err) { }

  bool fileDeleteSuccess;
  bool dbDeleteSuccess;
  QString errorText;
  model::Evidence model;
};
