#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <QStandardPaths>
#include <QString>

#include <iostream>
#include <QRegularExpression>

class Constants {
 public:


  static QString releaseOwner() {
    return "google";
//    return QString("%1").arg(SOURCE_CONTROL_OWNER);
  }

  static QString releaseRepo() {
    return "go-github";
//    return QString("%1").arg(SOURCE_CONTROL_REPO);;
  }

  static QString commitHash() {
    return QString("%1").arg(COMMIT_HASH);
  }

  static QString releaseTag() {
    static QString parsedReleaseTag = "";
    if (parsedReleaseTag.isEmpty()) {
      QRegularExpression compiledTagRegex("(?:tags/)?v(.*)");
      auto rawVersion = QString("%1").arg(VERSION_TAG);
      QRegularExpressionMatch match = compiledTagRegex.match(rawVersion);
      if (match.hasMatch()) {
        parsedReleaseTag = "v" + match.captured(1);
      }
      else {
        parsedReleaseTag = "v0.0.0-unversioned";
      }
    }

    return parsedReleaseTag;
  }
};


#endif // CONSTANTS_H
