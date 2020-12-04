#include "servers_list.h"

#include <QMessageBox>

#include "appservers.h"


ServersList::ServersList(QWidget* parent) : QWidget(parent) {
  buildUi();
  wireUi();
}

ServersList::~ServersList() {
  delete connectionsList;
  delete deleteButton;
  delete addButton;

  delete gridLayout;
}

void ServersList::buildUi() {
  gridLayout = new QGridLayout(this);
  gridLayout->setMargin(0);

  auto simpleButton = [this](QString name, int maxWidth = 0, int minWidth = 0){
    QPushButton* btn = new QPushButton(name, this);
    if (maxWidth > 0) {
      btn->setMaximumWidth(maxWidth);
      if(minWidth > 0) {
        btn->setMinimumWidth(minWidth);
      }
      btn->resize(maxWidth, btn->height());
    }
    btn->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    return std::move(btn);
  };

  deleteButton = simpleButton("-", 40, 25);
  deleteButton->setEnabled(false); // start disabled, since no item is selected
  addButton = simpleButton("+", 40, 25);

  connectionsList = new QListWidget(this);
  connectionsList->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);
  connectionsList->setSelectionMode(QAbstractItemView::ExtendedSelection);


  // Layout
  /*        0                 1
       +---------------+-------------+
       |                             |
    0  |   Connection List           |
       |                             |
       +---------------+-------------+
    1  | :Add Btn      | Delete Btn: |
       +---------------+-------------+
  */

  // row 0
  gridLayout->addWidget(connectionsList, 0, 0, 1, 2);

  // row 1
  gridLayout->addWidget(addButton, 1, 0);
  gridLayout->addWidget(deleteButton, 1, 1, Qt::AlignRight);

  this->setLayout(gridLayout);
}

void ServersList::wireUi() {
  auto btnClicked = &QPushButton::clicked;
  connect(addButton, btnClicked, this, &ServersList::addClicked);
  connect(deleteButton, btnClicked, this, &ServersList::deleteClicked);
  connect(connectionsList, &QListWidget::itemSelectionChanged, this, &ServersList::onItemSelectionChanged);
}

void ServersList::addClicked() {
  ServerItem s("New Connection", "", "", "");
  AppServers::getInstance().addServer(s);

  repopulateTable();
  connectionsList->setCurrentRow(connectionsList->count() - 1);
  emit onServerAdded(s);
}

void ServersList::deleteClicked() {
  auto selectedItems = connectionsList->selectedItems();
  if (selectedItems.length() == 0) {
    return;
  }

  // do the same action for all selected items, based on the first selection
  auto firstItem = qvariant_cast<ServerItem>(selectedItems[0]->data(Qt::UserRole));
  bool doDelete = !firstItem.deleted;

  if (doDelete) {
    QString deleteMsg = "Are you sure you want to delete %1?";
    deleteMsg = deleteMsg.arg(selectedItems.length() == 1
                                  ? "this server"
                                  : QString("these %1 servers").arg(selectedItems.size()) );
    auto reply = QMessageBox::question(this, "Delete connections?", deleteMsg);
    if (reply != QMessageBox::Yes) {
      return;
    }
  }
  std::vector<ServerItem> affectedUUIDs;
  for(auto selectedItem : selectedItems) {
    auto data = qvariant_cast<ServerItem>(selectedItem->data(Qt::UserRole));
    affectedUUIDs.push_back(data);
    auto affectServersFn = doDelete ? &AppServers::purgeServer : &AppServers::restoreServer;

    (AppServers::getInstance().*affectServersFn)(data.getServerUuid());
  }
  auto fn = doDelete ? &ServersList::onServersDeleted : &ServersList::onServersRestored;
  emit (this->*fn)(affectedUUIDs);

  repopulateTable();
}

void ServersList::onItemSelectionChanged() {
  auto selectedItems = connectionsList->selectedItems();
  std::vector<ServerItem> selectedServers;
  for (auto item : selectedItems) {
    auto data = qvariant_cast<ServerItem>(item->data(Qt::UserRole));
    selectedServers.push_back(data);
  }

  deleteButton->setEnabled(selectedServers.size() > 0);
  emit onServerSelectionChanged(selectedServers);
}

void ServersList::repopulateTable() {
  connectionsList->clear();
  std::vector<ServerItem> serverList = AppServers::getInstance().getServers();
  for (auto server : serverList) {
    auto item = buildServerItem(server);
    connectionsList->addItem(item);
  }
}

QListWidgetItem* ServersList::buildServerItem(ServerItem server) {
  auto item = new QListWidgetItem();
  auto castedData = QVariant::fromValue(server);
  item->setData(Qt::UserRole, castedData);
  item->setText(server.serverName);
  if( server.deleted ) {
    auto font = item->font();
    font.setStrikeOut(true);
    item->setFont(font);
  }

  return item;
}

void ServersList::clearSelection() {
  connectionsList->clearSelection();
}

void ServersList::refreshList() {
  repopulateTable();
}

