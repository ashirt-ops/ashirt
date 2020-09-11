// Copyright 2020, Verizon Media
// Licensed under the terms of MIT. See LICENSE file in project root for terms.

#ifndef GETINFO_H
#define GETINFO_H

#include <QGridLayout>
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
  void buildUi();
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
  DatabaseConnection *db;
  qint64 evidenceID;

  QNetworkReply *uploadAssetReply = nullptr;

  // Actions
  QAction* closeWindowAction = nullptr;

  // Ui Components
  QGridLayout* gridLayout;
  QPushButton* deleteButton;
  EvidenceEditor *evidenceEditor;
  LoadingButton *submitButton;

};

#endif  // GETINFO_H
