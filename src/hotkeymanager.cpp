#include "hotkeymanager.h"
#include "appconfig.h"

HotkeyManager::HotkeyManager()
  : m_hotkeyManager(new UGlobalHotkeys(this))
{
  connect(m_hotkeyManager, &UGlobalHotkeys::activated, this, &HotkeyManager::hotkeyTriggered);
}

HotkeyManager::~HotkeyManager() { delete m_hotkeyManager; }

void HotkeyManager::regKey(QString combo, GlobalHotkeyEvent evt)
{
  if (combo.isEmpty())
    return;
  registerKey(combo, evt);
}

void HotkeyManager::registerKey(const QString& binding, GlobalHotkeyEvent evt) {
  get()->m_hotkeyManager->registerHotkey(binding, size_t(evt));
}

void HotkeyManager::unregisterKey(GlobalHotkeyEvent evt) {
  get()->m_hotkeyManager->unregisterHotkey(size_t(evt));
}

void HotkeyManager::hotkeyTriggered(size_t hotkeyIndex) {
  if (hotkeyIndex == static_cast<size_t>(GlobalHotkeyEvent::ACTION_CAPTURE_AREA)) {
    Q_EMIT get()->captureAreaHotkeyPressed();
  }
  else if (hotkeyIndex == static_cast<size_t>(GlobalHotkeyEvent::ACTION_CAPTURE_WINDOW)) {
    Q_EMIT get()->captureWindowHotkeyPressed();
  }
  else if (hotkeyIndex == static_cast<size_t>(GlobalHotkeyEvent::ACTION_CAPTURE_CLIPBOARD)) {
    Q_EMIT get()->clipboardHotkeyPressed();
  }
}

void HotkeyManager::disableHotkeys() {
  get()->m_hotkeyManager->unregisterAllHotkeys();
}

void HotkeyManager::enableHotkeys() {
  get()->updateHotkeys();
}

void HotkeyManager::updateHotkeys() {
  get()->m_hotkeyManager->unregisterAllHotkeys();
  get()->regKey(AppConfig::value(CONFIG::SHORTCUT_SCREENSHOT),
                GlobalHotkeyEvent::ACTION_CAPTURE_AREA);
  get()->regKey(AppConfig::value(CONFIG::SHORTCUT_CAPTUREWINDOW),
                GlobalHotkeyEvent::ACTION_CAPTURE_WINDOW);
  get()->regKey(AppConfig::value(CONFIG::SHORTCUT_CAPTURECLIPBOARD),
                GlobalHotkeyEvent::ACTION_CAPTURE_CLIPBOARD);
}
