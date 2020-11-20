#include "connection_editorv2.h"

#include <QVariant>
#include <iostream>

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
  delete includeDeletedCheckbox;

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
  includeDeletedCheckbox = new QCheckBox("Include Deleted", this);
  includeDeletedCheckbox->setChecked(false);
  includeDeletedCheckbox->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

  connectionEditArea = new ConnectionProperties(this);
  connectionEditArea->setEnabled(false); // start disabled, since no item is selected
  connectionsList = new QListWidget(this);
  connectionsList->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::MinimumExpanding);

  // Layout
  /*        0                 1             2              3
       +---------------+-------------+-------------+--------------------+
    0  | Add Btn       | Delete Btn  + Incl Deleted|                    |
       +---------------+-------------+-------------+                    +
    1  |      Connection Selector                  |     Connection     |
       |              Menu                         |     Properties     |
       |                                           |                    |
       +---------------+-------------+-------------+--------------------+
  */

  // row 0
  gridLayout->addWidget(addButton, 0, 0);
  gridLayout->addWidget(deleteButton, 0, 1);
  gridLayout->addWidget(includeDeletedCheckbox, 0, 2);

  // row 0 + 1
  gridLayout->addWidget(connectionEditArea, 0, 3, 3, 1);

  // row 1
  gridLayout->addWidget(connectionsList, 1, 0, 1, 3);

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
  connect(includeDeletedCheckbox, &QCheckBox::stateChanged, this, &ConnectionEditorV2::onIncludeDeletedStateChange);

  connect(connectionEditArea, &ConnectionProperties::onSave, this, [](ServerItem data){
    AppServers::getInstance().updateServer(data);
  });
}

void ConnectionEditorV2::showEvent(QShowEvent* evt) {
  QDialog::showEvent(evt);
  repopulateTable();
}

void ConnectionEditorV2::repopulateTable() {
  connectionsList->clear();
  std::vector<ServerItem> serverList = AppServers::getInstance().getServers(includeDeletedCheckbox->isChecked());
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
  for(auto selectedItem : selectedItems) {
    auto data = qvariant_cast<ServerItem>(selectedItem->data(Qt::UserRole));

    doDelete
        ? AppServers::getInstance().deleteServer(data.getServerUuid())
        : AppServers::getInstance().restoreServer(data.getServerUuid());
  }

  repopulateTable();
  if( includeDeletedCheckbox->isChecked() ) {
    connectionsList->setCurrentRow(currentRow);
  }
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

void ConnectionEditorV2::onIncludeDeletedStateChange(int state) {
  Q_UNUSED(state);
  auto selectedItems = connectionsList->selectedItems();
  std::vector<QString> selectedUuids;

  for(auto item : selectedItems) {
    auto data = qvariant_cast<ServerItem>(item->data(Qt::UserRole));
    selectedUuids.push_back(data.getServerUuid());
  }
  repopulateTable();
  selectConnectionUuids(selectedUuids);
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
