#include "connection_properties.h"

ConnectionProperties::ConnectionProperties(QWidget *parent) : QWidget(parent) {
  buildUi();
  wireUi();
}

ConnectionProperties::~ConnectionProperties() {
  delete _nameLabel;
  delete _accessKeyLabel;
  delete _secretKeyLabel;
  delete _hostPathLabel;

  delete saveButton;
  delete connectionStatus;
  delete nameTextBox;
  delete accessKeyTextBox;
  delete secretKeyTextBox;
  delete hostPathTextBox;

  delete gridLayout;
}

void ConnectionProperties::buildUi() {
  gridLayout = new QGridLayout(this);
  gridLayout->setMargin(0);

  _nameLabel = new QLabel("Name");
  _hostPathLabel = new QLabel("Host Path");
  _accessKeyLabel = new QLabel("Access Key");
  _secretKeyLabel = new QLabel("Secret Key");
  _nameLabel->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
  _hostPathLabel->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
  _accessKeyLabel->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
  _secretKeyLabel->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

  nameTextBox = new QLineEdit(this);
  hostPathTextBox = new QLineEdit(this);
  accessKeyTextBox = new QLineEdit(this);
  secretKeyTextBox = new QLineEdit(this);
  connectionStatus = new ConnectionChecker(false, this);
  saveButton = new QPushButton("Save", this);
  saveButton->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

  // Layout
  /*        0                 1            2
       +---------------+-------------+-------------+
    0  | Name Lbl      | [Name TB]                 |
       +---------------+-------------+-------------+
    1  | Host Lbl      | [Host TB]                 |
       +---------------+-------------+-------------+
    2  | Access Lbl    | [Access TB]               |
       +---------------+-------------+-------------+
    3  | Secret Lbl    | [Secret TB]               |
       +---------------+-------------+-------------+
    4  | Test Connection Area                      |
       +---------------+-------------+-------------+
    5  | TODO: vertical Spacer                     |
       +---------------+-------------+-------------+
    6  | <None>        | <None>      | Save Btn    |
       +---------------+-------------+-------------+
  */

  int row = 0;

  // row 0
  gridLayout->addWidget(_nameLabel, row, 0);
  gridLayout->addWidget(nameTextBox, row, 1, 1, 2);
  row++;

  // row 1
  gridLayout->addWidget(_hostPathLabel, row, 0);
  gridLayout->addWidget(hostPathTextBox, row, 1, 1, 2);
  row++;

  // row 2
  gridLayout->addWidget(_accessKeyLabel, row, 0);
  gridLayout->addWidget(accessKeyTextBox, row, 1, 1, 2);
  row++;

  // row 3
  gridLayout->addWidget(_secretKeyLabel, row, 0);
  gridLayout->addWidget(secretKeyTextBox, row, 1, 1, 2);
  row++;

  // row 4
  gridLayout->addWidget(connectionStatus, row, 0, 1, 3);
  row++;

  // row 5
  gridLayout->addWidget(saveButton, row, 2);

  this->setLayout(gridLayout);
}

void ConnectionProperties::wireUi() {
  connect(saveButton, &QPushButton::clicked, this, &ConnectionProperties::onSaveClicked);

  connect(connectionStatus, &ConnectionChecker::pressed, this, &ConnectionProperties::onConnectionCheckerPressed);
}

void ConnectionProperties::onSaveClicked() {
  auto rtn = ServerItem(loadedItem.getId(), loadedItem.getServerUuid(), nameTextBox->text(),
                        accessKeyTextBox->text(), secretKeyTextBox->text(), hostPathTextBox->text(),
                        loadedItem.deleted);
  emit onSave(rtn);
}


void ConnectionProperties::loadItem(ServerItem item) {
  loadedItem = item;
  resetForm();
}

void ConnectionProperties::resetForm() {
  nameTextBox->setText(loadedItem.serverName);
  accessKeyTextBox->setText(loadedItem.accessKey);
  secretKeyTextBox->setText(loadedItem.secretKey);
  hostPathTextBox->setText(loadedItem.hostPath);
}

void ConnectionProperties::clearForm() {
  loadedItem = emptyItem;
  resetForm();
  connectionStatus->clearStatus();
}

bool ConnectionProperties::isDirty() {
  return (loadedItem.serverName != nameTextBox->text()) ||
         (loadedItem.accessKey != accessKeyTextBox->text()) ||
         (loadedItem.secretKey != secretKeyTextBox->text()) ||
         (loadedItem.hostPath != hostPathTextBox->text());
}

void ConnectionProperties::onConnectionCheckerPressed() {
  auto hostPath = hostPathTextBox->text();
  auto accessKey = accessKeyTextBox->text();
  auto secretKey = secretKeyTextBox->text();

  connectionStatus->setConnectionTestFields(hostPath, accessKey, secretKey);
}

void ConnectionProperties::setEnabled(bool enable) {
  nameTextBox->setEnabled(enable);
  hostPathTextBox->setEnabled(enable);
  accessKeyTextBox->setEnabled(enable);
  secretKeyTextBox->setEnabled(enable);
  connectionStatus->setEnabled(enable);
  saveButton->setEnabled(enable);
}
