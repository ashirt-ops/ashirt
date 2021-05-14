// Copyright 2020, Verizon Media
// Licensed under the terms of MIT. See LICENSE file in project root for terms.

#ifndef SETTINGS_H
#define SETTINGS_H

#include <QAction>
#include <QCloseEvent>
#include <QDialog>
#include <QDialogButtonBox>
#include <QErrorMessage>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QNetworkReply>
#include <QPushButton>
#include <QSpacerItem>
#include <QKeySequenceEdit>

#include "components/loading_button/loadingbutton.h"
#include "hotkeymanager.h"

/**
 * @brief The Settings class represents the settings dialog that displays when
 * a user chooses the "settings" option in the tray menu
 */
class Settings : public QDialog {
  Q_OBJECT

 public:
  /**
   * @brief Settings constructs the settings menu. UI will be built and wired.
   * @param hotkeyManager a handle to the HotkeyManager, so hotkeys may be updated upon saving.
   * @param parent
   */
  explicit Settings(HotkeyManager* hotkeyManager, QWidget* parent = nullptr);
  ~Settings();

 private:
  /// buildUi creates the window structure.
  void buildUi();
  /// wireUi connects the components to each other.
  void wireUi();
  /// onSaveClicked handles the save operation. Used when the "Save" button is pressed.
  void onSaveClicked();
  /// onCancelClicked handles the discard operation. Used when the "Cancel" button is pressed.
  void onCancelClicked();

  /// onClearShortcutsClicked handles the clear shortcuts button action.
  void onClearShortcutsClicked();

 public slots:
  /// showEvent extends the native showEvent handler. Restores the UI to system values.
  void showEvent(QShowEvent* evt) override;
  /// closeEvent extends the native closeEvent handler. Saves data prior to closing.
  void closeEvent(QCloseEvent* event) override;

  /// onTestConnectionClicked acts upon the "test connection" button press. Checks the network.
  void onTestConnectionClicked();
  /// onTestRequestComplete handles the network result action.
  void onTestRequestComplete();
  /// onBrowseClicked triggers when the "browse" button is pressed. Shows a file dialog to the user.
  void onBrowseClicked();

 private:
  /// hotkeyManager is a (shared) reference to the HotkeyManager. Not to be deleted.
  HotkeyManager* hotkeyManager;

  QNetworkReply* currentTestReply = nullptr;
  QAction* closeWindowAction = nullptr;

  // UI components
  QGridLayout* gridLayout = nullptr;
  QLabel* _eviRepoLabel = nullptr;
  QLabel* _accessKeyLabel = nullptr;
  QLabel* _secretKeyLabel = nullptr;
  QLabel* _hostPathLabel = nullptr;
  QLabel* _captureAreaCmdLabel = nullptr;
  QLabel* _captureAreaShortcutLabel = nullptr;
  QLabel* _captureWindowCmdLabel = nullptr;
  QLabel* _captureWindowShortcutLabel = nullptr;
  QLabel* _recordCodeblockShortcutLabel = nullptr;
  QLabel* connStatusLabel = nullptr;

  QLineEdit* eviRepoTextBox = nullptr;
  QLineEdit* accessKeyTextBox = nullptr;
  QLineEdit* secretKeyTextBox = nullptr;
  QLineEdit* hostPathTextBox = nullptr;
  QLineEdit* captureAreaCmdTextBox = nullptr;
  QKeySequenceEdit* captureAreaShortcutTextBox = nullptr;
  QLineEdit* captureWindowCmdTextBox = nullptr;
  QKeySequenceEdit* captureWindowShortcutTextBox = nullptr;
  QKeySequenceEdit* recordCodeblockShortcutTextBox = nullptr;
  LoadingButton* testConnectionButton = nullptr;
  QPushButton* eviRepoBrowseButton = nullptr;
  QPushButton* clearHotkeysButton = nullptr;
  QDialogButtonBox* buttonBox = nullptr;

  QErrorMessage* couldNotSaveSettingsMsg = nullptr;
  QSpacerItem* spacer = nullptr;
};

#endif  // SETTINGS_H
