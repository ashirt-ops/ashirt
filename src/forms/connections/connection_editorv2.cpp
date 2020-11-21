#include "connection_editorv2.h"

#include <QVariant>
#include <iostream>
#include <QMessageBox>

#include "appservers.h"

ConnectionEditorV2::ConnectionEditorV2(QWidget *parent) : QDialog(parent) {
  buildUi();
  wireUi();
}

ConnectionEditorV2::~ConnectionEditorV2() {
  delete connectionEditArea;
  delete connectionsList;
  delete deleteButton;
  delete addButton;

  delete closeWindowAction;
  delete gridLayout;
}

void ConnectionEditorV2::buildUi() {
  gridLayout = new QGridLayout(this);

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

  connectionEditArea = new ConnectionProperties(this);
  connectionEditArea->setEnabled(false); // start disabled, since no item is selected
  connectionsList = new QListWidget(this);
  connectionsList->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);
  connectionsList->setSelectionMode(QAbstractItemView::ExtendedSelection);

  // Layout
  /*        0                 1              2
       +---------------+-------------+--------------------+
    0  | Add Btn       | Delete Btn  |                    |
       +---------------+-------------+                    +
    1  |      Connection Selector    |     Connection     |
       |              Menu           |     Properties     |
       |                             |                    |
       +---------------+-------------+--------------------+
  */

  // row 0
  gridLayout->addWidget(addButton, 0, 0);
  gridLayout->addWidget(deleteButton, 0, 1, Qt::AlignRight);

  // row 0 + 1
  gridLayout->addWidget(connectionEditArea, 0, 2, 2, 1);

  // row 1
  gridLayout->addWidget(connectionsList, 1, 0, 1, 2);

  // adjust how the dialog will expand -- we want the +/- buttons to always align with the list area
  gridLayout->setColumnStretch(0, 0);
  gridLayout->setColumnStretch(1, 0);
  gridLayout->setColumnStretch(2, 1);
  gridLayout->setRowStretch(0, 0);
  gridLayout->setRowStretch(1, 1);

  closeWindowAction = new QAction(this);
  closeWindowAction->setShortcut(QKeySequence::Close);
  this->addAction(closeWindowAction);

  this->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);
  this->resize(800, 1);
  this->setWindowTitle("Connection Editor");
  this->setLayout(gridLayout);
}

void ConnectionEditorV2::wireUi() {
  auto btnClicked = &QPushButton::clicked;

  connect(addButton, btnClicked, this, &ConnectionEditorV2::addClicked);
  connect(deleteButton, btnClicked, this, &ConnectionEditorV2::deleteClicked);
  connect(connectionsList, &QListWidget::itemSelectionChanged, this, &ConnectionEditorV2::onItemSelectionChanged);

  connect(connectionEditArea, &ConnectionProperties::onSave, [this](ServerItem data){
    // we can only save a single item, so only one row is selected
    int currentRow = connectionsList->currentRow();
    AppServers::getInstance().updateServer(data);
    repopulateTable();
    connectionsList->setCurrentRow(currentRow);
  });

}

void ConnectionEditorV2::showEvent(QShowEvent* evt) {
  QDialog::showEvent(evt);
  connectionsList->clearSelection();
  repopulateTable();
}

void ConnectionEditorV2::repopulateTable() {
  connectionsList->clear();
  std::vector<ServerItem> serverList = AppServers::getInstance().getServers();
  for (auto server : serverList) {
    auto item = buildServerItem(server);
    connectionsList->addItem(item);
  }
}

QListWidgetItem* ConnectionEditorV2::buildServerItem(ServerItem server) {
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

void ConnectionEditorV2::addClicked() {
  ServerItem s("New Connection", "", "", "");
  AppServers::getInstance().addServer(s);

  repopulateTable();
  connectionsList->setCurrentRow(connectionsList->count() - 1);
  connectionEditArea->highlightNameTextbox();
}

void ConnectionEditorV2::deleteClicked() {
  int currentRow = connectionsList->currentRow();
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

  for(auto selectedItem : selectedItems) {
    auto data = qvariant_cast<ServerItem>(selectedItem->data(Qt::UserRole));

    doDelete
        ? AppServers::getInstance().purgeServer(data.getServerUuid())
        : AppServers::getInstance().restoreServer(data.getServerUuid());
  }

  repopulateTable();
}

void ConnectionEditorV2::onItemSelectionChanged() {
  auto selectedItems = connectionsList->selectedItems();
  if (selectedItems.length() == 0) {
    connectionEditArea->clearForm();
    connectionEditArea->setEnabled(false);
    deleteButton->setEnabled(false);
    return;
  }
  else if( selectedItems.length() > 1) {
    connectionEditArea->setEnabled(false);
  }
  else {
    connectionEditArea->setEnabled(true);
  }

  deleteButton->setEnabled(true);
  QListWidgetItem* item = selectedItems[0];
  auto data = qvariant_cast<ServerItem>(item->data(Qt::UserRole));
  connectionEditArea->loadItem(data);
}

void ConnectionEditorV2::selectConnectionUuids(std::vector<QString> targetUuids, bool firstOnly) {
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
