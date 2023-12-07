#pragma once

#include <QObject>

#include "helpers/hotkeys/uglobalhotkeys.h"

/**
 * @brief The HotkeyManager class registers and unregisters hotkeys against the operating system.
 * This class does so in a best-effort manner. As such, registering hotkeys may not always work.
 */
class HotkeyManager : public QObject {
  Q_OBJECT
  /// GlobalHotkeyEvent provides names for all possible application-global hotkeys

 public:
  enum GlobalHotkeyEvent {
    // Reserving 1 (UGlobalHotkey default)
    ACTION_CAPTURE_AREA = 2,
    ACTION_CAPTURE_WINDOW = 3,
    ACTION_CAPTURE_CLIPBOARD = 4,
  };

  static HotkeyManager* get() {
    static HotkeyManager m;
    return &m;
  }
  /**
   * @brief registerKey pairs a given key combination with a given event type.
   * @param binding A string representing the actual command (e.g. Alt+f1)
   * @param evt A GlobalHotkeyEvent specifying what should happen when a key is pressed.
   */
  static void registerKey(const QString& binding, GlobalHotkeyEvent evt);
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

 private slots:
  /// hotkeyTriggered provides a slot for interacting with the underlying UGlobalHotkey manager.
  static void hotkeyTriggered(size_t hotkeyIndex);

 private:  
  HotkeyManager();
  ~HotkeyManager();
  /// Interal Reg method used to filter Empty keys
  void regKey(QString combo, GlobalHotkeyEvent evt);
  /// hotkeyManager is a reference to the raw hotkey manager, which a 3rd party manages.
  UGlobalHotkeys* m_hotkeyManager = nullptr;
};
