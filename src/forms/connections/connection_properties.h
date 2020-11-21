#ifndef CONNECTIONPROPERTIES_H
#define CONNECTIONPROPERTIES_H

#include <QGridLayout>
#include <QLabel>
#include <QObject>
#include <QPushButton>
#include <QLineEdit>
#include <QWidget>
#include <QSpacerItem>

#include "components/connection_checker/connectionchecker.h"
#include "config/server_item.h"

class ConnectionProperties : public QWidget {
  Q_OBJECT

 public:
  ConnectionProperties(QWidget* parent = nullptr);
  ~ConnectionProperties();

 private:
  /// buildUi constructs the window structure.
  void buildUi();
  /// wireUi connects UI elements together
  void wireUi();

  /// resetForm sets all fields to the loaded serverItem
  void resetForm();

 private slots:
  void onConnectionCheckerPressed();

  /// fires when the save button is clicked
  void onSaveClicked();

 public:
  /// isDirty is used to track if the user has made changes.
  /// True indicates that unsaved data is present on this form
  bool isDirty();
  void clearForm();
  void setEnabled(bool enable);

 signals:
  void onSave(ServerItem data);

 public slots:
  void loadItem(ServerItem item);


 private:
  ServerItem emptyItem;
  ServerItem loadedItem;

  // UI Elements
  QGridLayout* gridLayout = nullptr;

  QLabel* _nameLabel = nullptr;
  QLabel* _accessKeyLabel = nullptr;
  QLabel* _secretKeyLabel = nullptr;
  QLabel* _hostPathLabel = nullptr;

  QPushButton* saveButton = nullptr;
  ConnectionChecker* connectionStatus = nullptr;
  QLineEdit* nameTextBox = nullptr;
  QLineEdit* accessKeyTextBox = nullptr;
  QLineEdit* secretKeyTextBox = nullptr;
  QLineEdit* hostPathTextBox = nullptr;
  QSpacerItem* spacer = nullptr;
};

#endif // CONNECTIONPROPERTIES_H
