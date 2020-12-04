#include "connection_editor.h"

#include <QVariant>
#include <iostream>
#include <QMessageBox>

#include "appservers.h"

ConnectionEditor::ConnectionEditor(QWidget *parent) : QDialog(parent) {
  buildUi();
  wireUi();
}

ConnectionEditor::~ConnectionEditor() {
  delete connectionEditArea;
  delete connectionsList;
  delete deleteButton;
  delete addButton;

  delete closeWindowAction;
  delete gridLayout;
}

void ConnectionEditor::buildUi() {
  gridLayout = new QGridLayout(this);

  connectionEditArea = new ConnectionProperties(this);
  connectionEditArea->setEnabled(false); // start disabled, since no item is selected

  serversList = new ServersList(this);

  // Layout
  /*        0                 1
       +---------------+--------------+
    0  |  Servers List |  Connection  |
       |               |  Properties  |
       +---------------+--------------+
  */

  // row 0
  gridLayout->addWidget(serversList, 0, 0);
  gridLayout->addWidget(connectionEditArea, 0, 1);

  gridLayout->setColumnStretch(0, 1);
  gridLayout->setColumnStretch(1, 3);

  closeWindowAction = new QAction(this);
  closeWindowAction->setShortcut(QKeySequence::Close);
  this->addAction(closeWindowAction);

  this->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);
  this->resize(800, 1);
  this->setWindowTitle("Connection Editor");
  this->setLayout(gridLayout);
}

void ConnectionEditor::wireUi() {
  connect(connectionEditArea, &ConnectionProperties::onSave, this, &ConnectionEditor::onConnectionSaved);

  connect(serversList, &ServersList::onServerAdded, [this](ServerItem s){
    connectionEditArea->highlightNameTextbox();
  });

  connect(serversList, &ServersList::onServersDeleted, [this](std::vector<ServerItem> affectedServers){
    std::cout << "(deleted) Not yet implemented";
  });
  connect(serversList, &ServersList::onServersRestored, [this](std::vector<ServerItem> affectedServers){
    std::cout << "(restored) Not yet implemented";
  });
  connect(serversList, &ServersList::onServerSelectionChanged, [this](std::vector<ServerItem> selectedRows){
    connectionEditArea->setEnabled(selectedRows.size() == 1);
    if (selectedRows.size() == 0) {
      connectionEditArea->clearForm();
      return;
    }
    connectionEditArea->loadItem(selectedRows[0]);
  });
}

void ConnectionEditor::showEvent(QShowEvent* evt) {
  QDialog::showEvent(evt);
  serversList->clearSelection();
  serversList->refreshList();
}

void ConnectionEditor::selectConnectionUuids(std::vector<QString> targetUuids, bool firstOnly) {
  auto allItems = connectionsList->findItems("", Qt::MatchContains);

  for(auto item : allItems) {
    auto data = qvariant_cast<ServerItem>(item->data(Qt::UserRole));
    for (auto targetUuid : targetUuids) {
      if( data.getServerUuid() == targetUuid ) {
        connectionsList->setCurrentItem(item);
        if( firstOnly ) {
          return;
        }
      }
    }
  }
}

void ConnectionEditor::onConnectionSaved(ServerItem item) {
  AppServers::getInstance().updateServer(item);
}
