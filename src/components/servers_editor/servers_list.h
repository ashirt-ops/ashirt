#ifndef SERVERS_LIST_H
#define SERVERS_LIST_H

#include <QGridLayout>
#include <QListWidget>
#include <QListWidgetItem>
#include <QPushButton>
#include <QWidget>

#include "config/server_item.h"

class ServersList : public QWidget {
  Q_OBJECT

 public:
  ServersList(QWidget* parent = nullptr);
  ~ServersList();

 private:
  /// buildUi constructs the window structure.
  void buildUi();
  /// wireUi connects UI elements together
  void wireUi();

  void repopulateTable();

  QListWidgetItem* buildServerItem(ServerItem server);

 public:
  void clearSelection();
  void refreshList();

 signals:
  void onServerAdded(ServerItem s);
  void onServersDeleted(std::vector<ServerItem>);
  void onServersRestored(std::vector<ServerItem>);
  void onServerSelectionChanged(std::vector<ServerItem>);

 private slots:
  void addClicked();
  void deleteClicked();
  void onItemSelectionChanged();

 private:

  // UI Elements
  QGridLayout* gridLayout = nullptr;

  QListWidget* connectionsList = nullptr;
  QPushButton* addButton = nullptr;
  QPushButton* deleteButton = nullptr;
};

#endif // SERVERS_LIST_H
