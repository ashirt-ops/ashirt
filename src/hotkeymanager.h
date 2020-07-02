// Copyright 2020, Verizon Media
// Licensed under the terms of GPLv3. See LICENSE file in project root for terms.

#ifndef HOTKEYMANAGER_H
#define HOTKEYMANAGER_H

#include <QObject>

#include "helpers/screenshot.h"
#include "tools/UGlobalHotkey/uglobalhotkeys.h"

/**
 * @brief The HotkeyManager class registers and unregisters hotkeys against the operating system.
 * This class does so in a best-effort manner. As such, registering hotkeys may not always work.
 */
class HotkeyManager : public QObject {
  Q_OBJECT

 public:
  HotkeyManager(Screenshot* ss);
  ~HotkeyManager();

  /// GlobalHotkeyEvent provides names for all possible application-global hotkeys
  enum GlobalHotkeyEvent {
    // Reserving 1 (UGlobalHotkey default)
    ACTION_CAPTURE_AREA = 2,
    ACTION_CAPTURE_WINDOW = 3,
    ACTION_CAPTURE_CODEBLOCK = 4,
  };

 public:
  /**
   * @brief registerKey pairs a given key combination with a given event type.
   * @param binding A string representing the actual command (e.g. Alt+f1)
   * @param evt A GlobalHotkeyEvent specifying what should happen when a key is pressed.
   */
  void registerKey(const QString& binding, GlobalHotkeyEvent evt);
  /// unregisterKey removes the handling specified for the given event. Safe to call even if
  /// no key has been registered.
  void unregisterKey(GlobalHotkeyEvent evt);

 signals:
  /// codeblockHotkeyPressed signals when the ACTION_CAPTURE_CODEBLOCK event has been triggered.
  void codeblockHotkeyPressed();

 public slots:
  /// updateHotkeys retrives AppConfig data to set known global hotkeys. Removes _all_ (Application)
  /// hotkeys when called.
  void updateHotkeys();

 private slots:
  /// hotkeyTriggered provides a slot for interacting with the underlying UGlobalHotkey manager.
  void hotkeyTriggered(size_t hotkeyIndex);

 private:
  /// screenshotTool is a (shared) reference to the ScreenShot object. Not to be deleted.
  Screenshot* screenshotTool;
  /// hotkeyManager is a reference to the raw hotkey manager, which a 3rd party manages.
  UGlobalHotkeys* hotkeyManager;
};

#endif  // HOTKEYMANAGER_H
