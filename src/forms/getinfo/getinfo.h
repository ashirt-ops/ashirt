// Copyright 2020, Verizon Media
// Licensed under the terms of MIT. See LICENSE file in project root for terms.

#ifndef GETINFO_H
#define GETINFO_H

#include <QAction>
#include <QDialog>
#include <QNetworkReply>

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

  void showEvent(QShowEvent *evt) override;

 private slots:
  void submitButtonClicked();
  void deleteButtonClicked();

  void onUploadComplete();

 public:
 signals:
  void evidenceSubmitted(model::Evidence evidence);

 private:
  Ui::GetInfo *ui;
  DatabaseConnection *db;
  qint64 evidenceID;

  EvidenceEditor *evidenceEditor;
  QAction* closeWindowAction = nullptr;

  QNetworkReply *uploadAssetReply = nullptr;
  LoadingButton *loadingButton;
};

#endif  // GETINFO_H
