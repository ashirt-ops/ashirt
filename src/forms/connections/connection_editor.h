#ifndef CONNECTIONEDITOR_H
#define CONNECTIONEDITOR_H

#include <QAction>
#include <QDialog>
#include <QGridLayout>
#include <QLabel>
#include <QObject>
#include <QPushButton>
#include <QTextEdit>
#include <QListWidget>

#include "components/connection_checker/connectionchecker.h"
#include "connection_properties.h"
#include "config/server_item.h"

class ConnectionEditor : public QDialog {
  Q_OBJECT

 public:
  ConnectionEditor(QWidget *parent = nullptr);
  ~ConnectionEditor();

 private:
  /// buildUi constructs the window structure.
  void buildUi();

  /// wireUi connects UI elements together
  void wireUi();

  void showEvent(QShowEvent* evt) override;
  void repopulateTable();
  QListWidgetItem* buildServerItem(ServerItem server);
  void selectConnectionUuids(std::vector<QString> targetUuids, bool firstOnly=false);

 private slots:
  void addClicked();
  void deleteClicked();
  void onItemSelectionChanged();


 private:
  QAction* closeWindowAction = nullptr;

  // UI Elements
  QGridLayout* gridLayout = nullptr;


  QListWidget* connectionsList = nullptr;
  QPushButton* addButton = nullptr;
  QPushButton* deleteButton = nullptr;
  ConnectionProperties* connectionEditArea = nullptr;
};

#endif // CONNECTIONEDITOR_H
