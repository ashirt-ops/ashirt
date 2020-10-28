#ifndef CONNECTION_CELL_DATA_H
#define CONNECTION_CELL_DATA_H

#include <QString>
#include <QVariant>
#include <QDataStream>

enum CellType {
  CELL_TYPE_INVALID,
  CELL_TYPE_NORMAL,
  CELL_TYPE_DELETE,
  CELL_TYPE_ADD,
};

enum CellFlag {
  CELL_FLAG_NONE = 0,
  CELL_FLAG_ERROR = 2 << 0,
  CELL_FLAG_WARNING = 2 << 1,
};

static CellFlag worseOf(CellFlag a, CellFlag b) {
  if (a == CELL_FLAG_ERROR || b == CELL_FLAG_ERROR) {
    return CELL_FLAG_ERROR;
  }
  if (a == CELL_FLAG_WARNING || b == CELL_FLAG_WARNING) {
    return CELL_FLAG_WARNING;
  }
  return CELL_FLAG_NONE;
};

class ConnectionCellData {

 public:
  ConnectionCellData() = default;
  ~ConnectionCellData() = default;
  ConnectionCellData(const ConnectionCellData &) = default;

  ConnectionCellData(QString id, QString data, CellType startingType = CELL_TYPE_NORMAL) {
    this->serverUuid = id;
    this->originalData = data;
    this->cellType = startingType;
  }

  bool isOriginalData(QString update) {
    return update == originalData;
  }

  void updateData(QString update) {
    originalData = update;
  }

  void markDeleted(bool deleted) {
    cellType = deleted ? CELL_TYPE_DELETE : CELL_TYPE_NORMAL;
  }

  /// setFlags changes the stored flags to be the provided flags
  void setFlags(unsigned int flags) { cellFlags = flags; }

  /// addFlag adjusts the stored flags to include the passed flags
  void addFlag(unsigned int flags) { cellFlags |= flags; }

  /// removeFlag adjusts the stored flags to remove the passed flags
  void removeFlag(unsigned int flags) { cellFlags &= ~(flags); }

  /// getFlags returns the stored flags
  unsigned int getFlags() { return cellFlags; }

  bool hasError() { return (CELL_FLAG_ERROR & cellFlags) > 0; }
  bool hasWarning() { return (CELL_FLAG_WARNING & cellFlags) > 0; }

  bool isMarkedDeleted() { return cellType == CELL_TYPE_DELETE; }
  bool isMarkedToAdd() { return cellType == CELL_TYPE_ADD; }

 public:
  QString originalData = "";
  QString serverUuid = "";
  CellType cellType = CELL_TYPE_NORMAL;
  unsigned int cellFlags = 0;
};

Q_DECLARE_METATYPE(ConnectionCellData);

#endif // CONNECTION_CELL_DATA_H
