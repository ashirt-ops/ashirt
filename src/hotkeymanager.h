#pragma once

#include <QObject>

class QHotkey;

/**
 * @brief The HotkeyManager class registers and unregisters hotkeys against the operating system.
 * This class does so in a best-effort manner. As such, registering hotkeys may not always work.
 */
class HotkeyManager : public QObject {
  Q_OBJECT
  /// GlobalHotkeyEvent provides names for all possible application-global hotkeys

 public:
  enum class GlobalHotkeyEvent {
    ACTION_CAPTURE_AREA,
    ACTION_CAPTURE_WINDOW,
    ACTION_CAPTURE_CLIPBOARD,
  };

  static HotkeyManager* get() {
    static HotkeyManager m;
    return &m;
  }

  /// hotkeysSupported reports whether the current platform/session can register global hotkeys.
  /// Returns false on platforms QHotkey cannot grab keys on (notably native Wayland sessions),
  /// where registration is a no-op.
  static bool hotkeysSupported();

  /// unregisterKey removes the handling specified for the given event. Safe to call even if
  /// no key has been registered.
  static void unregisterKey(GlobalHotkeyEvent evt);

  /// disableHotkeys removes all of the keybindings.
  static void disableHotkeys();

  /// enableHotkeys "restores" all of the currently set hotkeys. This acts as the counterpoint to
  /// disableHotkeys, but functionally is identical to updateHotKeys.
  static void enableHotkeys();

 signals:
  /// clipboardHotkeyPressed signals when the ACTION_CAPTURE_CLIPBOARD event has been triggered.
  void clipboardHotkeyPressed();
  /// captureWindowHotkeyPressed signals when the ACTION_CAPTURE_WINDOW event has been triggered.
  void captureWindowHotkeyPressed();
  /// captureAreaHotkeyPressed signals when the ACTION_CAPTURE_AREA event has been triggered.
  void captureAreaHotkeyPressed();

 public slots:
  /// updateHotkeys retrives AppConfig data to set known global hotkeys. Removes _all_ (Application)
  /// hotkeys when called.
  static void updateHotkeys();

 private:
  HotkeyManager();
  ~HotkeyManager();
  /// hotkeyFor returns the QHotkey backing the given event, or nullptr if unknown.
  QHotkey* hotkeyFor(GlobalHotkeyEvent evt) const;
  /// setShortcut (re)binds the QHotkey for evt to combo. An empty combo unregisters it.
  void setShortcut(GlobalHotkeyEvent evt, const QString& combo);

  /// One QHotkey per capture action. Each QHotkey::activated wires straight to the matching signal.
  QHotkey* m_captureArea = nullptr;
  QHotkey* m_captureWindow = nullptr;
  QHotkey* m_captureClipboard = nullptr;
};
