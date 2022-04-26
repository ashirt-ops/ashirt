// Copyright 2020, Verizon Media
// Licensed under the terms of MIT. See LICENSE file in project root for terms.

#pragma once

#include <stdexcept>

class BadDatabaseStateError : public std::runtime_error {
 public:
  BadDatabaseStateError() : std::runtime_error("Database is in an inconsistent state") {}
};

class DBDriverUnavailableError : public std::runtime_error {
 public:
  DBDriverUnavailableError(std::string friendlyDriverName)
      : std::runtime_error(friendlyDriverName + " driver is unavailable") {}
};
