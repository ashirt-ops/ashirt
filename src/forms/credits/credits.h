// Copyright 2020, Verizon Media
// Licensed under the terms of MIT. See LICENSE file in project root for terms.

#pragma once

#include "ashirtdialog/ashirtdialog.h"
#include "dtos/github_release.h"
#include "helpers/constants.h"

class QLabel;

class Credits : public AShirtDialog {
  Q_OBJECT

 public:
  explicit Credits(QWidget *parent = nullptr);
  ~Credits() = default;

 public slots:
  void onReleasesUpdate(bool success, QList<dto::GithubRelease> releases);

 private:
  /// updateRelease waits for an update on a new version, if any. If called, and a new version does exist
  /// this will update the updateLabel to have a link to the new release
  void updateRelease();

 private:
  QLabel* updateLabel = nullptr;
  dto::ReleaseDigest updateDigest;
  inline static const auto attribBLOB_splitChar = QStringLiteral(">");
  inline static const auto attrib_splitChar = QStringLiteral("~");
  inline static const auto hyperlinkMd = QStringLiteral("[%1](%2)");
  inline static const auto creditLine = QStringLiteral("| %1 | %2 | %3 |\n");
  inline static const QString baseUpdateText = QStringLiteral("A new update is available! Click <a href=\"%1\">here</a> for more details.");
  inline static const auto attribBLOB
      = QStringLiteral(
              "Qt~http://qt.io~The Qt Company~LGPLv3~https://www.gnu.org/licenses/lgpl-3.0.html"
              ">QProgressIndicator~https://github.com/mojocorp/QProgressIndicator~Mojocorp, et al~MIT License~https://github.com/mojocorp/QProgressIndicator/blob/master/LICENSE"
              ">MultipartEncoder~https://github.com/AndsonYe/MultipartEncoder~Ye Yangang~MIT License~https://github.com/AndsonYe/MultipartEncoder/blob/master/LICENSE"
              ">UGlobalHotkey~https://github.com/joelatdeluxe/UGlobalHotkey~Anton Konstantinov, et al~Public Domain~https://fairuse.stanford.edu/overview/public-domain/welcome"
              ">AspectRatioPixmapLabel~https://stackoverflow.com/a/22618496/4262552~phyatt, et al~CC BY-SA 3.0~https://creativecommons.org/licenses/by-sa/3.0/"
        );
  inline static const auto preambleMarkdown
      = QStringLiteral("Version: %1 \n\n"
                   "Commit Hash: %2\n\n"
                   "Copyright %3, Verizon Media\n\n"
                   "Licensed under the terms of %4\n\n"
                   "A short user guide can be found %5\n\n"
                   "Report issues %6\n\n")
                   .arg(Constants::releaseTag()
                   , Constants::commitHash()
                   , QStringLiteral("2020 - %1").arg(QDateTime::currentDateTime().date().year())
                   , hyperlinkMd.arg(QStringLiteral("MIT"), QStringLiteral("https://github.com/theparanoids/ashirt/blob/master/LICENSE"))
                   , hyperlinkMd.arg(QStringLiteral("here"), Constants::userGuideUrl())
                   , hyperlinkMd.arg(QStringLiteral("here"), Constants::reportAnIssueUrl()));

  static QString attributionMarkdown() {
    const QStringList attribs = attribBLOB.split(attribBLOB_splitChar);
    auto msg =
        QStringLiteral("This application uses the following open source software:\n\n"
        "| Project/Library | Authors | License |\n"
        "| --------------- | ------- | ------- |\n");
    for (const auto& attrib: attribs) {
        const QStringList credit = attrib.split(attrib_splitChar);
        auto library = hyperlinkMd.arg(credit.at(0), credit.at(1));
        auto copyright = hyperlinkMd.arg(credit.at(3), credit.at(4));
        msg.append(creditLine.arg(library, credit.at(2), copyright));
    }
    return msg;
  }
  inline static const auto normalBodyMarkdown = QStringLiteral("# ASHIRT\n\n %1 ## Credits\n %2").arg(preambleMarkdown, attributionMarkdown());
};
