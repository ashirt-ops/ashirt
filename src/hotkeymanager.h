// Copyright 2020, Verizon Media
// Licensed under the terms of GPLv3. See LICENSE file in project root for terms.

#ifndef HOTKEYMANAGER_H
#define HOTKEYMANAGER_H

#include <QObject>

#include "helpers/screenshot.h"
#include "tools/UGlobalHotkey/uglobalhotkeys.h"

// HotkeyManager is a singleton for managing global hotkeys.
class HotkeyManager : public QObject {
  Q_OBJECT

 public:
  HotkeyManager(Screenshot* ss);
  ~HotkeyManager();

  enum GlobalHotkeyEvent {
    // Reserving 1 (UGlobalHotkey default)
    ACTION_CAPTURE_AREA = 2,
    ACTION_CAPTURE_WINDOW = 3
  };

 private:
  Screenshot* screenshotTool;
  UGlobalHotkeys* hotkeyManager;

 public:
  void registerKey(const QString& binding, GlobalHotkeyEvent evt);
  void unregisterKey(GlobalHotkeyEvent evt);

 public slots:
  void updateHotkeys();

 private slots:
  void hotkeyTriggered(size_t hotkeyIndex);
};

#endif  // HOTKEYMANAGER_H
