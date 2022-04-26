// Copyright 2020, Verizon Media
// Licensed under the terms of MIT. See LICENSE file in project root for terms.

#pragma once

#include <QDateTime>
#include <QString>

#include "tag.h"

namespace model {
class Evidence {
 public:
  qint64 id;
  QString path;
  QString operationSlug;
  QString description;
  QString errorText;
  QString contentType;
  QDateTime recordedDate;
  QDateTime uploadDate;
  std::vector<Tag> tags;
};
}  // namespace model
