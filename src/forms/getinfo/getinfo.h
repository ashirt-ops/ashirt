// Copyright 2020, Verizon Media
// Licensed under the terms of GPLv3. See LICENSE file in project root for terms.

#ifndef GETINFO_H
#define GETINFO_H

#include <QDialog>
#include <QNetworkReply>
#include <QKeyEvent>

#include "components/evidence_editor/evidenceeditor.h"
#include "components/loading/qprogressindicator.h"
#include "components/loading_button/loadingbutton.h"
#include "db/databaseconnection.h"
#include "dtos/tag.h"

namespace Ui {
class GetInfo;
}

class GetInfo : public QDialog {
  Q_OBJECT

 public:
  explicit GetInfo(DatabaseConnection *db, qint64 evidenceID, QWidget *parent = nullptr);
  ~GetInfo();

 private:
  void wireUi();
  bool saveData();
  void setActionButtonsEnabled(bool enabled);

 public slots:
  void keyPressEvent(QKeyEvent *evt) override;

 private slots:
  void submitButtonClicked();
  void deleteButtonClicked();

  void onUploadComplete();

 private:
  Ui::GetInfo *ui;
  DatabaseConnection *db;
  qint64 evidenceID;

  EvidenceEditor *evidenceEditor;

  QNetworkReply *uploadAssetReply = nullptr;
  LoadingButton *loadingButton;
};

#endif  // GETINFO_H
