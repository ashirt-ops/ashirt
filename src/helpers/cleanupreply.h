// Licensed under the terms of MIT. See LICENSE file in project root for terms.

#pragma once

#include <QNetworkReply>

static void cleanUpReply(QNetworkReply **reply) {
  if (*reply == nullptr) {
    return;
  }

  (*reply)->isFinished() ? (*reply)->close() : (*reply)->abort();
  (*reply)->deleteLater();
  *reply = nullptr;
}
