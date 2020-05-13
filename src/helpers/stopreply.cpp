// Copyright 2020, Verizon Media
// Licensed under the terms of GPLv3. See LICENSE file in project root for terms.

#include "stopreply.h"

#include <QNetworkReply>

// stopReply aborts a request, then cleans up the reply (via deleteLater)
// also sets the reply pointer to nullptr. This is for use cases where the
// reply is to be ignored.
void stopReply(QNetworkReply **reply) {
  if (*reply == nullptr) {
    return;
  }

  (*reply)->abort();
  (*reply)->deleteLater();
  *reply = nullptr;
}

// tidyReply cleans up a "completed" reply, closing the connection and marking
// for deletion. Additionally, this sets the reply pointer to nullptr. This is
// for use cases where a reply has extracted all necessary information and clean-up is necessary.
void tidyReply(QNetworkReply **reply) {
  if (*reply == nullptr) {
    return;
  }

  (*reply)->close();
  (*reply)->deleteLater();
  *reply = nullptr;
}
