#ifndef DELETEEVIDENCERESPONSE_H
#define DELETEEVIDENCERESPONSE_H

#include <QString>

#include "models/evidence.h"

struct DeleteEvidenceResponse {
  DeleteEvidenceResponse(model::Evidence model) {
    this->model = model;
  }
  DeleteEvidenceResponse(bool fileDeleteSuccess, bool dbDeleteSuccess, QString err,
                         model::Evidence model)
      : DeleteEvidenceResponse(model) {
    this->fileDeleteSuccess = fileDeleteSuccess;
    this->dbDeleteSuccess = dbDeleteSuccess;
    this->errorText = err;
  }
  bool fileDeleteSuccess;
  bool dbDeleteSuccess;
  QString errorText;
  model::Evidence model;
};

#endif  // DELETEEVIDENCERESPONSE_H
