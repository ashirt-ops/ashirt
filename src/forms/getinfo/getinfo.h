#pragma once

#include "ashirtdialog/ashirtdialog.h"

#include <QNetworkReply>
#include "components/evidence_editor/evidenceeditor.h"

class DatabaseConnection;
class LoadingButton;

class GetInfo : public AShirtDialog {
  Q_OBJECT

 public:
  explicit GetInfo(DatabaseConnection *db, qint64 evidenceID, QWidget *parent = nullptr);
  ~GetInfo();

 private:
  void buildUi();
  void wireUi();
  bool saveData();
  void showEvent(QShowEvent *evt) override;

 signals:
  void setActionButtonsEnabled(bool enabled);
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

  // Ui Components
  EvidenceEditor *evidenceEditor = nullptr;
  LoadingButton *submitButton = nullptr;
};
