#ifndef CONNECTIONEDITOR_H
#define CONNECTIONEDITOR_H

#include <QAction>
#include <QDialog>
#include <QGridLayout>
#include <QLabel>
#include <QObject>
#include <QPushButton>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTextEdit>
#include <QWidget>
#include <functional>

#include "connection_cell_data.h"
#include "connections_table_analysis.h"
#include "components/connection_checker/connectionchecker.h"
#include "db/databaseconnection.h"

struct ConnectionRow {
  QTableWidgetItem* status;
  QTableWidgetItem* name;
  QTableWidgetItem* hostPath;
  QTableWidgetItem* accessKey;
  QTableWidgetItem* secretKey;
};

class ConnectionEditor : public QDialog {
  Q_OBJECT

 public:
  explicit ConnectionEditor(DatabaseConnection* db, QWidget *parent = nullptr);
  ~ConnectionEditor();

 private:
  /// buildUi constructs the window structure.
  void buildUi();
  /// buildTableUi constructs the connections table.
  void buildTableUi();

  /// wireUi connects UI elements together
  void wireUi();
  ///
  void populateTable();
  /// buildRow constructs a table row without adding it to the table
  ConnectionRow buildRow(QString id="", QString name="", QString apiUrl="", QString accessKey="", QString secretKey="", CellType cellType=CELL_TYPE_ADD);
  /// buildRow is a shorthand for constructing a table row from a (presumed to exist) server item
  /// see buildRow(QString name, ...)
  ConnectionRow buildRow(model::Server item);

  /// serializeRows converts the table rows into a vector for model::Server
  /// Note: serializeRows serializes the table as it is -- validateTable should generally called first.
  std::vector<model::Server> serializeRows();
  /// addNewRow appends the given ConnectionRow to the table. You can get this row by calling buildRow
  int addNewRow(ConnectionRow rowData);
  /// clearTable removes all of the content from the table, and sets the size to 0
  void clearTable();
  /// withNoSorting removes the sorting on the table prior to running func, then re-enables sorting
  void withNoSorting(std::function<void()> func);
  /// readCellData is shorthand for accessing the item's data on the given row/column, and converting it into a ConnectionCellData
  ConnectionCellData readCellData(int row, int col=0);
  /// updateCellData retrieves the cell located at (row, col)'s data and provides a reference to that data
  /// to the provided function which may update the data. This data is then set within the same cell data
  void updateCellData(int row, int column, std::function<void(ConnectionCellData* data)> update);
  /// getCell is a convenience method for calling connectionsTable->item(row, col)
  QTableWidgetItem* getCell(int row, int col);

  /// showEvent extends QDialog's showEvent. Resets the applied filters.
  void showEvent(QShowEvent* evt) override;

  QBrush changedBrush() {
    static QBrush brush = QBrush(changedColor);
    return brush;
  }
  QBrush addedBrush() {
    static QBrush brush = QBrush(addedColor);
    return brush;
  }
  QBrush deletedBrush() {
    static QBrush brush = QBrush(deletedColor);
    return brush;
  }
  QBrush warningBrush() {
    static QBrush brush = QBrush(warningBGColor);
    return brush;
  }
  QBrush errorBrush() {
    static QBrush brush = QBrush(errorBGColor);
    return brush;
  }
  QBrush normalBrush() {
    static QBrush brush = QBrush();
    return brush;
  }

 private:
  /// analyzeTable reviews the table and generates a report based on certain conditions
  ConnectionsTableAnalysis analyzeTable();
  /// markupTable updates the table with warning/status messages and removes empty rows, given
  /// an analysis (generally generated from analyzeTable())
  void markupTable(ConnectionsTableAnalysis analysis);


 private slots:
   void onCellChanged(int row, int column);
   void onCellClicked(int row, int column);

   void onConnectionCheckerPressed();
   /// onSaveClicked is called when the save button is pressed
   void onSaveClicked();
   /// onDeleteClicked is called when the "-" button is pressed
   void onDeleteClicked();
   /// onAddClicked is called when the "+" button is pressed
   void onAddClicked();

 private:
  /// db is a shared reference to the database. Do not delete
  DatabaseConnection* db = nullptr;

  QColor changedColor = QColor(0xFFFFCC);
  QColor addedColor = QColor(0xCCFFCC);
  QColor deletedColor = QColor(0xFFCCCC);
  QColor warningBGColor = QColor(0xFFFF80);
  QColor errorBGColor = QColor(0xFF8080);

  // UI Elements
  QGridLayout* gridLayout = nullptr;
  QAction* closeWindowAction = nullptr;

  QTableWidget* connectionsTable = nullptr;

  QPushButton* addButton = nullptr;
  QPushButton* saveButton = nullptr;
  QPushButton* deleteButton = nullptr;

  ConnectionChecker* connectionStatus = nullptr;
  QLabel* _logLabel = nullptr;
  QTextEdit* logTextBox = nullptr;
};

#endif // CONNECTIONEDITOR_H
