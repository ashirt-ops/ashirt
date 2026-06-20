// Licensed under the terms of MIT. See LICENSE file in project root for terms.

#pragma once

#include <QNetworkReply>

static void cleanUpReply(QNetworkReply **reply) {
  if (*reply == nullptr) {
    return;
  }

  // Take a local copy and clear the caller's pointer up front. abort() emits finished()
  // synchronously, which can re-enter this function via the reply's finished slot; clearing
  // first turns that nested call into a no-op instead of a double-free / null deref.
  QNetworkReply *r = *reply;
  *reply = nullptr;

  // Drop the finished slot so abort() can't re-trigger a handler that reads the now-closing
  // reply ("QIODevice::read: device not open").
  QObject::disconnect(r, nullptr, nullptr, nullptr);

  r->isFinished() ? r->close() : r->abort();
  r->deleteLater();
}
