#pragma once

#include <QDateTime>

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
  QList<Tag> tags;
};
}  // namespace model
