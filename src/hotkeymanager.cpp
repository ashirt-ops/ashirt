#include "hotkeymanager.h"

#include <QKeySequence>

#include "appconfig.h"
#include "helpers/hotkeys/qhotkey.h"

HotkeyManager::HotkeyManager()
  : m_captureArea(new QHotkey(this))
  , m_captureWindow(new QHotkey(this))
  , m_captureClipboard(new QHotkey(this))
{
  connect(m_captureArea, &QHotkey::activated, this, &HotkeyManager::captureAreaHotkeyPressed);
  connect(m_captureWindow, &QHotkey::activated, this, &HotkeyManager::captureWindowHotkeyPressed);
  connect(m_captureClipboard, &QHotkey::activated, this, &HotkeyManager::clipboardHotkeyPressed);
}

HotkeyManager::~HotkeyManager() = default;

QHotkey* HotkeyManager::hotkeyFor(GlobalHotkeyEvent evt) const {
  switch (evt) {
    case GlobalHotkeyEvent::ACTION_CAPTURE_AREA:
      return m_captureArea;
    case GlobalHotkeyEvent::ACTION_CAPTURE_WINDOW:
      return m_captureWindow;
    case GlobalHotkeyEvent::ACTION_CAPTURE_CLIPBOARD:
      return m_captureClipboard;
  }
  return nullptr;
}

void HotkeyManager::setShortcut(GlobalHotkeyEvent evt, const QString& combo) {
  QHotkey* hotkey = hotkeyFor(evt);
  if (!hotkey)
    return;
  // An empty combo resets the shortcut, leaving the hotkey unregistered.
  hotkey->setShortcut(QKeySequence::fromString(combo, QKeySequence::PortableText), true);
}

void HotkeyManager::unregisterKey(GlobalHotkeyEvent evt) {
  if (QHotkey* hotkey = get()->hotkeyFor(evt))
    hotkey->setRegistered(false);
}

void HotkeyManager::disableHotkeys() {
  get()->m_captureArea->setRegistered(false);
  get()->m_captureWindow->setRegistered(false);
  get()->m_captureClipboard->setRegistered(false);
}

void HotkeyManager::enableHotkeys() {
  updateHotkeys();
}

void HotkeyManager::updateHotkeys() {
  get()->setShortcut(GlobalHotkeyEvent::ACTION_CAPTURE_AREA, AppConfig::value(CONFIG::SHORTCUT_SCREENSHOT));
  get()->setShortcut(GlobalHotkeyEvent::ACTION_CAPTURE_WINDOW, AppConfig::value(CONFIG::SHORTCUT_CAPTUREWINDOW));
  get()->setShortcut(GlobalHotkeyEvent::ACTION_CAPTURE_CLIPBOARD, AppConfig::value(CONFIG::SHORTCUT_CAPTURECLIPBOARD));
}
