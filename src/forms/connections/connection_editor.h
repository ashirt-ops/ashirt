#ifndef CONNECTIONEDITOR_H
#define CONNECTIONEDITOR_H

#include <QAction>
#include <QDialog>
#include <QGridLayout>
#include <QObject>

#include "components/servers_editor/connection_properties.h"
#include "components/servers_editor/servers_list.h"
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

 private slots:
  void onConnectionSaved(ServerItem data);
  void serverSelectionChanged(std::vector<ServerItem> selectedServers);

 private:
  QAction* closeWindowAction = nullptr;

  // UI Elements
  QGridLayout* gridLayout = nullptr;

  ConnectionProperties* connectionEditArea = nullptr;
  ServersList* serversList = nullptr;
};

#endif // CONNECTIONEDITOR_H
