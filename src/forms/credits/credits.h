// Copyright 2020, Verizon Media
// Licensed under the terms of MIT. See LICENSE file in project root for terms.

#pragma once

#include "ashirtdialog/ashirtdialog.h"
#include "dtos/github_release.h"

class QLabel;

class Credits : public AShirtDialog {
  Q_OBJECT

 public:
  explicit Credits(QWidget *parent = nullptr);
  ~Credits() = default;

 public slots:
  void onReleasesUpdate(bool success, std::vector<dto::GithubRelease> releases);

 private:
  /// buildUi creates the window structure.
  void buildUi();
  /// wireUi connects the components to each other.
  void wireUi();
  /// updateRelease waits for an update on a new version, if any. If called, and a new version does exist
  /// this will update the updateLabel to have a link to the new release
  void updateRelease();

 private:
  QLabel* updateLabel = nullptr;
  dto::ReleaseDigest updateDigest;
  inline static const QString baseUpdateText = QStringLiteral("A new update is available! Click <a href=\"%1\">here</a> for more details.");
};
