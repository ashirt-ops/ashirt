// Copyright 2020, Verizon Media
// Licensed under the terms of MIT. See LICENSE file in project root for terms.

#include "hotkeymanager.h"
#include "appconfig.h"

HotkeyManager::HotkeyManager(QObject *parent)
  : QObject (parent)
  , hotkeyManager(new UGlobalHotkeys(this))
{
  connect(hotkeyManager, &UGlobalHotkeys::activated, this, &HotkeyManager::hotkeyTriggered);
}

HotkeyManager::~HotkeyManager() { delete hotkeyManager; }

void HotkeyManager::registerKey(const QString& binding, GlobalHotkeyEvent evt) {
  hotkeyManager->registerHotkey(binding, size_t(evt));
}

void HotkeyManager::unregisterKey(GlobalHotkeyEvent evt) {
  hotkeyManager->unregisterHotkey(size_t(evt));
}

void HotkeyManager::hotkeyTriggered(size_t hotkeyIndex) {
  if (hotkeyIndex == ACTION_CAPTURE_AREA) {
    Q_EMIT captureAreaHotkeyPressed();
  }
  else if (hotkeyIndex == ACTION_CAPTURE_WINDOW) {
    Q_EMIT captureWindowHotkeyPressed();
  }
  else if (hotkeyIndex == ACTION_CAPTURE_CLIPBOARD) {
    Q_EMIT clipboardHotkeyPressed();
  }
}

void HotkeyManager::disableHotkeys() {
  hotkeyManager->unregisterAllHotkeys();
}

void HotkeyManager::enableHotkeys() {
  updateHotkeys();
}

void HotkeyManager::updateHotkeys() {
  hotkeyManager->unregisterAllHotkeys();
  auto regKey = [this](QString combo, GlobalHotkeyEvent evt) {
    if (!combo.isEmpty()) {
      registerKey(combo, evt);
    }
  };
  regKey(AppConfig::value(CONFIG::SHORTCUT_SCREENSHOT), ACTION_CAPTURE_AREA);
  regKey(AppConfig::value(CONFIG::SHORTCUT_CAPTUREWINDOW), ACTION_CAPTURE_WINDOW);
  regKey(AppConfig::value(CONFIG::SHORTCUT_CAPTURECLIPBOARD), ACTION_CAPTURE_CLIPBOARD);
}
