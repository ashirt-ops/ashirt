#include "connection_editor.h"

#include "appservers.h"

ConnectionEditor::ConnectionEditor(QWidget *parent) : QDialog(parent) {
  buildUi();
  wireUi();
}

ConnectionEditor::~ConnectionEditor() {
  delete connectionEditArea;
  delete serversList;

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
  connect(serversList, &ServersList::onServerSelectionChanged, this, &ConnectionEditor::serverSelectionChanged);

  connect(serversList, &ServersList::onServerAdded, [this](){ connectionEditArea->highlightNameTextbox(); });
}

void ConnectionEditor::showEvent(QShowEvent* evt) {
  QDialog::showEvent(evt);
  serversList->clearSelection();
  serversList->refreshList();
}

void ConnectionEditor::onConnectionSaved(ServerItem item) {
  AppServers::getInstance().updateServer(item);
}

void ConnectionEditor::serverSelectionChanged(std::vector<ServerItem> selectedServers) {
  connectionEditArea->setEnabled(selectedServers.size() == 1);
  if (selectedServers.size() == 0) {
    connectionEditArea->clearForm();
    return;
  }
  connectionEditArea->loadItem(selectedServers[0]);

}
