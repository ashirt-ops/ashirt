#ifndef IMPORTDIALOG_H
#define IMPORTDIALOG_H

#include <QAction>
#include <QCheckBox>
#include <QDialog>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QProgressBar>
#include <QPushButton>
#include <QWidget>

#include "db/databaseconnection.h"
#include "porting/system_manifest.h"

class PortingDialog : public QDialog {
  Q_OBJECT

 public:
  enum PortType {
    Import,
    Export
  };

 public:
  explicit PortingDialog(PortType dialogType, DatabaseConnection* db, QWidget *parent = nullptr);
  ~PortingDialog();

 private:
  /// buildUi creates the window structure.
  void buildUi();
  /// wireUi connects the components to each other.
  void wireUi();

  void onSubmitPressed();
  void onBrowsePresed();

  void doExport(porting::SystemManifest* manifest, QString exportPath);
  porting::SystemManifest* doPreImport(QString pathToSystemManifest);
  void doImport(porting::SystemManifest* manifest);
  void onPortComplete(bool success);

 signals:
  void onWorkComplete(bool success);

 private:
  DatabaseConnection* db; // borrowed
  /// executedManifest contains a pointer to the system manifest used to import/export data
  /// Saved so that it can be cleaned up post-execution
  porting::SystemManifest* executedManifest = nullptr;

  PortType dialogType;
  QAction* closeWindowAction = nullptr;

  // UI Components
  QGridLayout* gridLayout = nullptr;
  QLabel* _selectFileLabel = nullptr;
  QLabel* portStatusLabel = nullptr;

  QPushButton* submitButton = nullptr;
  QPushButton* browseButton = nullptr;
  QLineEdit* pathTextBox = nullptr;
  QProgressBar* progressBar = nullptr;
  QCheckBox* portConfigCheckBox = nullptr;
  QCheckBox* portEvidenceCheckBox = nullptr;
};

#endif // IMPORTDIALOG_H
