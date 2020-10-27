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
#include <QPushButton>
#include <QSpacerItem>
#include <QKeySequenceEdit>

#include "components/connection_checker/connectionchecker.h"
#include "db/databaseconnection.h"
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
  explicit Settings(DatabaseConnection* db, HotkeyManager* hotkeyManager, QWidget* parent = nullptr);
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

 public slots:
  /// showEvent extends the native showEvent handler. Restores the UI to system values.
  void showEvent(QShowEvent* evt) override;
  /// closeEvent extends the native closeEvent handler. Saves data prior to closing.
  void closeEvent(QCloseEvent* event) override;

  /// onCheckConnectionPressed acts upon the "test connection" button press.
  void onCheckConnectionPressed();
  void onBrowseClicked();

 private:
  /// hotkeyManager is a (shared) reference to the HotkeyManager. Not to be deleted.
  HotkeyManager* hotkeyManager;
  /// db is a shared reference to the database. Do not delete
  DatabaseConnection* db;

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

  QLineEdit* eviRepoTextBox = nullptr;
  QLineEdit* accessKeyTextBox = nullptr;
  QLineEdit* secretKeyTextBox = nullptr;
  QLineEdit* hostPathTextBox = nullptr;
  QLineEdit* captureAreaCmdTextBox = nullptr;
  QKeySequenceEdit* captureAreaShortcutTextBox = nullptr;
  QLineEdit* captureWindowCmdTextBox = nullptr;
  QKeySequenceEdit* captureWindowShortcutTextBox = nullptr;
  QKeySequenceEdit* recordCodeblockShortcutTextBox = nullptr;
  QPushButton* eviRepoBrowseButton = nullptr;
  QDialogButtonBox* buttonBox = nullptr;
  ConnectionChecker* connectionStatus = nullptr;

  QErrorMessage* couldNotSaveSettingsMsg = nullptr;
  QSpacerItem* spacer = nullptr;
};

#endif  // SETTINGS_H
