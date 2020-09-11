// Copyright 2020, Verizon Media
// Licensed under the terms of MIT. See LICENSE file in project root for terms.

#ifndef CREDITS_H
#define CREDITS_H

#include <QAction>
#include <QDialog>
#include <QTextBrowser>
#include <QDialogButtonBox>
#include <QGridLayout>

#include "dtos/github_release.h"

class Credits : public QDialog {
  Q_OBJECT

 public:
  explicit Credits(QWidget *parent = nullptr);
  ~Credits();

 public slots:
  void onReleasesUpdate(bool success, std::vector<dto::GithubRelease> releases);

 private:
  /// buildUi creates the window structure.
  void buildUi();
  /// wireUi connects the components to each other.
  void wireUi();
  void updateBody();

 private:
  QAction* closeWindowAction = nullptr;

  // UI Components
  QGridLayout* gridLayout = nullptr;
  QTextBrowser* creditsArea = nullptr;
  QDialogButtonBox* buttonBox = nullptr;

  dto::ReleaseDigest updateDigest;

};

#endif  // CREDITS_H
