#include "connection_editor.h"

#include <QHeaderView>
#include <QJsonDocument>
#include <iostream>
#include <unordered_map>
#include <set>

enum ColumnIndexes {
  COL_STATUS = 0,
  COL_NAME,
  COL_API_URL,
  COL_ACCESS_KEY,
  COL_SECRET_KEY,
};

static QStringList columnNames() {
  static QStringList names;
  if (names.count() == 0) {
    // Note: these should always be in the order defined by columnIndexes
    names.insert(COL_STATUS, " "); // relative width: 1
    names.insert(COL_NAME, "Name"); //relative width: 2.5
    names.insert(COL_API_URL, "Host Path"); //relative width: 5
    names.insert(COL_ACCESS_KEY, "Access Key"); //relative width: 5
    names.insert(COL_SECRET_KEY, "Secret Key"); //relative width: 15
  }
  return names;
}

ConnectionEditor::ConnectionEditor(DatabaseConnection* db, QWidget *parent) : QDialog(parent) {
  this->db = db;
  buildUi();
  wireUi();
}

ConnectionEditor::~ConnectionEditor() {
  delete connectionsTable;

  delete saveButton;
  delete deleteButton;
  delete addButton;

  delete connectionStatus;

  delete logTextBox;
  delete _logLabel;

  delete closeWindowAction;
  delete gridLayout;
}

void ConnectionEditor::buildUi() {
  gridLayout = new QGridLayout(this);

  buildTableUi();

  logTextBox = new QTextEdit(this);
  logTextBox->setReadOnly(true);

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

  saveButton = simpleButton("Save");
  deleteButton = simpleButton("-", 40, 25);
  addButton = simpleButton("+", 40, 25);

  connectionStatus = new ConnectionChecker(this);

  _logLabel = new QLabel("Connection Notes", this);

  // Layout
  /*        0                 1           2             3             4
       +---------------+-------------+------------+-------------+-------------+
    0  | Add Btn       | Delete Btn  | <None>     | <None>      | Save Btn    |
       +---------------+-------------+------------+-------------+-------------+
    1  |                                                                      |
       |                     Content Table                                    |
       |                                                                      |
       +---------------+-------------+------------+-------------+-------------+
    2  |                        Loading Status                                |
       +---------------+-------------+------------+-------------+-------------+
    3  | log Label                                                            |
       +---------------+-------------+------------+-------------+-------------+
    4  |                                                                      |
       |                       Error Log                                      |
       |                                                                      |
       +---------------+-------------+------------+-------------+-------------+
  */

  // row 0
  gridLayout->addWidget(deleteButton, 0, 0);
  gridLayout->addWidget(addButton, 0, 1);
  gridLayout->addWidget(saveButton, 0, 4);

  // row 1
  gridLayout->addWidget(connectionsTable, 1, 0, 1, gridLayout->columnCount());

  // row 2
  gridLayout->addWidget(connectionStatus, 2, 0, 1, gridLayout->columnCount());

  // row 3
  gridLayout->addWidget(_logLabel, 3, 0, 1, gridLayout->columnCount());

  // row 4
  gridLayout->addWidget(logTextBox, 4, 0, 1, gridLayout->columnCount());

  closeWindowAction = new QAction(this);
  closeWindowAction->setShortcut(QKeySequence::Close);
  this->addAction(closeWindowAction);

  this->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);
  this->resize(800, 600);
  this->setWindowTitle("Connection Editor");
  this->setLayout(gridLayout);
}

void ConnectionEditor::wireUi() {
  auto btnClicked = &QPushButton::clicked;

  connect(saveButton, btnClicked, this, &ConnectionEditor::onSaveClicked);
  connect(deleteButton, btnClicked, this, &ConnectionEditor::onDeleteClicked);
  connect(addButton, btnClicked, this, &ConnectionEditor::onAddClicked);

  connect(connectionStatus, &ConnectionChecker::pressed, this, &ConnectionEditor::onConnectionCheckerPressed);

  connect(connectionsTable, &QTableWidget::cellChanged, this, &ConnectionEditor::onCellChanged);
  connect(connectionsTable, &QTableWidget::cellClicked, this, &ConnectionEditor::onCellClicked);
}

void ConnectionEditor::onConnectionCheckerPressed() {
  auto hostPath = getCell(connectionsTable->currentRow(), COL_API_URL)->text();
  auto accessKey = getCell(connectionsTable->currentRow(), COL_ACCESS_KEY)->text();
  auto secretKey = getCell(connectionsTable->currentRow(), COL_SECRET_KEY)->text();

  connectionStatus->setConnectionTestFields(hostPath, accessKey, secretKey);
}

void ConnectionEditor::showEvent(QShowEvent* evt) {
  QDialog::showEvent(evt);
  connectionStatus->clearStatus();
  clearTable();
  populateTable();
}

void ConnectionEditor::clearTable() {
  withNoSorting([this](){
    connectionsTable->clearContents();
    connectionsTable->setRowCount(0);
  });
}

void ConnectionEditor::onSaveClicked() {
  auto analysis = analyzeTable();
  markupTable(analysis);
  if (connectionsTable->currentRow() > -1) {
    onCellClicked(connectionsTable->currentRow(), connectionsTable->currentColumn()); // trigger log update
  }

  if (!analysis.hasNoErrors()) {
    return;
  }

  auto data = serializeRows();
  for (auto datum : data) {
    if (datum.serverUuid == "") {
      datum.serverUuid = model::Server::newUUID();
      db->createServer(datum);
    }
    else {
      db->updateFullServerDetails(datum.serverName, datum.accessKey,
                                  datum.secretKey, datum.hostPath, datum.serverUuid);
    }
  }

  // code below iterates through non-error rows, and resets the data to match the current value
  // this ignores error rows, in case we want to apply partial saves/successes
  for(int rowIndex = 0; rowIndex < connectionsTable->rowCount(); rowIndex++) {
    if (readCellData(rowIndex).hasError()) {
      continue;
    }
    for(int colIndex = 0; colIndex < connectionsTable->columnCount(); colIndex++) {
      updateCellData(rowIndex, colIndex, [this, rowIndex, colIndex](ConnectionCellData* item){
        item->updateData(getCell(rowIndex, colIndex)->text());
      });
    }
  }
}

void ConnectionEditor::onDeleteClicked() {
  int selectedRow = connectionsTable->currentRow();
  if (selectedRow < 0 || selectedRow >= connectionsTable->rowCount() ) {
    return;
  }

  if (readCellData(selectedRow).isMarkedToAdd()) {
    withNoSorting([this, selectedRow](){ connectionsTable->removeRow(selectedRow); });
  }
  else {
    for (int colIndex = 0; colIndex < connectionsTable->columnCount(); colIndex ++) {
      updateCellData(selectedRow, colIndex, [](ConnectionCellData* data){
        data->markDeleted(!data->isMarkedDeleted()); // toggle deleted flag
      });
    }
  }
}

void ConnectionEditor::onAddClicked() {
  withNoSorting([this](){
    addNewRow(buildRow());
  });
}

int ConnectionEditor::addNewRow(ConnectionRow rowData) {
  int newRowIndex = connectionsTable->rowCount();
  connectionsTable->setRowCount(newRowIndex + 1);

  connectionsTable->setItem(newRowIndex, COL_STATUS, rowData.status);
  connectionsTable->setItem(newRowIndex, COL_NAME, rowData.name);
  connectionsTable->setItem(newRowIndex, COL_API_URL, rowData.hostPath);
  connectionsTable->setItem(newRowIndex, COL_ACCESS_KEY, rowData.accessKey);
  connectionsTable->setItem(newRowIndex, COL_SECRET_KEY, rowData.secretKey);

  return newRowIndex;
}

void ConnectionEditor::onCellChanged(int row, int column) {
  auto widget = getCell(row, column);
  auto data = qvariant_cast<ConnectionCellData>(widget->data(Qt::UserRole));

  if (column == COL_STATUS) {
    if (data.hasError()) {
      widget->setBackground(errorBrush());
      widget->setText("❗");
      return;
    }
    else if (data.hasWarning()) {
      widget->setBackground(warningBrush());
      widget->setText("⚠");
      return;
    }
    else {
      widget->setText("");
    }
  }

  if (data.isMarkedToAdd()) {
    widget->setBackground(addedBrush());
  }
  else if (data.isMarkedDeleted()) {
    widget->setBackground(deletedBrush());
  }
  else if (!data.isOriginalData(widget->text())) {
    widget->setBackground(changedBrush());
  }
  else {
    widget->setBackground(normalBrush());
  }
}

void ConnectionEditor::onCellClicked(int row, int column) {
  Q_UNUSED(column);

  logTextBox->setText(getCell(row, COL_STATUS)->toolTip());
}

void ConnectionEditor::buildTableUi() {
  connectionsTable = new QTableWidget(this);

  QStringList colNames = columnNames();
  connectionsTable->setColumnCount(colNames.length());
  connectionsTable->setHorizontalHeaderLabels(colNames);
  connectionsTable->setSelectionMode(QAbstractItemView::SingleSelection);
  connectionsTable->setSortingEnabled(true);
  connectionsTable->verticalHeader()->setVisible(false);
  connectionsTable->horizontalHeader()->setCascadingSectionResizes(false);
  connectionsTable->horizontalHeader()->setStretchLastSection(true);
  connectionsTable->horizontalHeader()->setSortIndicatorShown(true);
  connectionsTable->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

void ConnectionEditor::populateTable() {
  withNoSorting([this](){
    for (auto item : db->getServers()) {
      addNewRow(buildRow(item));
    }
  });
}

ConnectionRow ConnectionEditor::buildRow(model::Server item) {
  return buildRow(item.serverUuid, item.serverName, item.hostPath, item.accessKey, item.secretKey, CELL_TYPE_NORMAL);
}

ConnectionRow ConnectionEditor::buildRow(QString id, QString name, QString apiUrl, QString accessKey, QString secretKey, CellType cellType) {
  ConnectionRow row{};

  auto basicItem = [id, cellType](QString text) -> QTableWidgetItem* {
    auto item = new QTableWidgetItem();
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable);
    item->setText(text);
    item->setData(Qt::UserRole, QVariant::fromValue(ConnectionCellData(id, text, cellType)));
    return item;
  };

  row.status = basicItem("");
  row.status->setFlags(row.status->flags() ^ Qt::ItemIsEditable);
  row.status->setTextAlignment(Qt::AlignCenter);
  row.name = basicItem(name);
  row.hostPath = basicItem(apiUrl);
  row.accessKey = basicItem(accessKey);
  row.secretKey = basicItem(secretKey);
  return row;
}

std::vector<model::Server> ConnectionEditor::serializeRows() {
  int rowCount = connectionsTable->rowCount();
  std::vector<model::Server> updatedData;

  for (int rowIndex = 0; rowIndex < rowCount; rowIndex++) {
    QString uuid = readCellData(rowIndex, COL_NAME).serverUuid; // any cell will work, just choosing name here.
    QString name = getCell(rowIndex, COL_NAME)->text().trimmed();
    QString apiURL = getCell(rowIndex, COL_API_URL)->text().trimmed();
    QString accessKey = getCell(rowIndex, COL_ACCESS_KEY)->text().trimmed();
    QString secretKey = getCell(rowIndex, COL_SECRET_KEY)->text().trimmed();

    updatedData.emplace_back(model::Server(uuid, name, accessKey, secretKey, apiURL));
  }

  return updatedData;
}

ConnectionsTableAnalysis ConnectionEditor::analyzeTable() {
  ConnectionsTableAnalysis rtn;

  std::unordered_map<QString, int> serverNameMap;
  auto duplicateRow = [](size_t a, size_t b){ return std::pair<size_t, size_t>(a, b); };

  std::vector<model::Server> rowsData = serializeRows();

  for (size_t rowIndex = 0; rowIndex < rowsData.size(); rowIndex++) {
    auto row = rowsData[rowIndex];

    // check if rows are empty
    if (row.isEmpty()) {
      rtn.emptyRows.push_back(rowIndex);
      continue; // no need to check the other stuff
    }

    // check if serverName is empty
    if (row.serverName.isEmpty()) {
      rtn.noServerNameRows.push_back(rowIndex);
    }
    else if( !row.isComplete() ) { // check if rows are missing data
      rtn.incompleteDataRows.push_back(rowIndex);
    }

    // check if rows have the same name
    auto foundIndex = serverNameMap.find(row.serverName);
    if (foundIndex == serverNameMap.end()) {
      serverNameMap.emplace(row.serverName, rowIndex);
    }
    else {
      rtn.duplicateServerNames.push_back(duplicateRow(rowIndex, foundIndex->second));
    }

  }

  return rtn;
}

void ConnectionEditor::markupTable(ConnectionsTableAnalysis analysis) {
  std::vector<std::pair<QStringList, CellFlag>> markup;

  // reset current analysis + prep markup results
  for (int rowIndex = 0; rowIndex < connectionsTable->rowCount(); rowIndex++) {
    getCell(rowIndex, COL_STATUS)->setToolTip("");
    updateCellData(rowIndex, COL_STATUS, [](ConnectionCellData* data){
      data->setFlags(CELL_FLAG_NONE);
    });
    markup.emplace_back(std::pair<QStringList, CellFlag>(QStringList(), CELL_FLAG_NONE));
  }

  for (auto rowIndex : analysis.incompleteDataRows) {
    markup[rowIndex].first.append("Warning: This entry is incomplete. Communication will not be possible.");
    markup[rowIndex].second = worseOf(markup[rowIndex].second, CELL_FLAG_WARNING);
  }

  for (auto rowIndex : analysis.noServerNameRows) {
    markup[rowIndex].first.append("Error: Server name must be set to save this entry.");
    markup[rowIndex].second = worseOf(markup[rowIndex].second, CELL_FLAG_ERROR);
  }

  // determine all duplicate rows, then mark each as a duplicate
  std::set<size_t> duplicateRows;
  for (auto rowIndexPair : analysis.duplicateServerNames) {
    duplicateRows.emplace(rowIndexPair.first);
    duplicateRows.emplace(rowIndexPair.second);
  }
  for( auto rowIndex : duplicateRows ) {
    markup[rowIndex].first.append("Error: Server name must be unique to save this entry.");
    markup[rowIndex].second = worseOf(markup[rowIndex].second, CELL_FLAG_ERROR);
  }

  for (size_t rowIndex = 0; rowIndex < markup.size(); rowIndex++) {
    updateCellData(rowIndex, COL_STATUS, [markup, rowIndex](ConnectionCellData* data){
      data->addFlag(markup[rowIndex].second);
    });
    getCell(rowIndex, COL_STATUS)->setToolTip(markup[rowIndex].first.join("\n"));
  }

  // iterate through list of empty rows (in reverse) to remove from the table
  // this should always be last to ensure other indexes are correct
  for ( int i = analysis.emptyRows.size() - 1; i >= 0; i-- ) {
    connectionsTable->removeRow(analysis.emptyRows[i]);
  }
}

QTableWidgetItem* ConnectionEditor::getCell(int row, int col) {
  return connectionsTable->item(row, col);
}

ConnectionCellData ConnectionEditor::readCellData(int row, int col) {
  auto item = getCell(row, col);
  if (item != nullptr) {
    auto data = item->data(Qt::UserRole);
    if (data.isValid()) {
      return qvariant_cast<ConnectionCellData>(data);
    }
  }
  return ConnectionCellData();
}

void ConnectionEditor::updateCellData(int row, int column, std::function<void(ConnectionCellData* data)> update) {
  auto item = getCell(row, column);
  ConnectionCellData cellData = readCellData(row, column);
  update(&cellData);
  item->setData(Qt::UserRole, QVariant::fromValue(cellData));
}

void ConnectionEditor::withNoSorting(std::function<void ()> func) {
  connectionsTable->setSortingEnabled(false);
  func();
  connectionsTable->setSortingEnabled(true);
}
