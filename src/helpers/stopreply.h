// Copyright 2020, Verizon Media
// Licensed under the terms of GPLv3. See LICENSE file in project root for terms.

#ifndef STOPREPLY_H
#define STOPREPLY_H

#include <QNetworkReply>

void stopReply(QNetworkReply **reply);
void tidyReply(QNetworkReply **reply);

#endif  // STOPREPLY_H
