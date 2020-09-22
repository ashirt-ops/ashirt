#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <QStandardPaths>
#include <QString>

#include <iostream>
#include <QRegularExpression>

class Constants {
 public:
  static QString UnknownRepoValue() {
    return "???";
  }
  static QString UnknownOwnerValue() {
    return "???";
  }

  static QString configLocation() {
    return QStandardPaths::writableLocation(QStandardPaths::DataLocation) + "/config.json";
  }

  static QString dbLocation() {
    return QStandardPaths::writableLocation(QStandardPaths::DataLocation) + "/evidence.sqlite";
  }

  static QString defaultEvidenceRepo() {
    return QStandardPaths::writableLocation(QStandardPaths::DataLocation) + "/evidence";
  }

  static QString releaseOwner() {
    return parseRepo(RepoField::owner);
  }

  static QString releaseRepo() {
    return parseRepo(RepoField::repo);
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
      parsedReleaseTag = match.hasMatch() ? "v" + match.captured(1) : "v0.0.0-unversioned";
    }

    return parsedReleaseTag;
  }
 private:
  enum RepoField {
    owner = 0,
    repo = 1,
  };

  static QString parseRepo(RepoField field) {
    static QString parsedRepo = "";
    static QString parsedOwner = "";

    if (parsedRepo == "") {
      auto rawRepo = QString("%1").arg(SOURCE_CONTROL_REPO);
      QRegularExpression ownerRegex("^([^/]+)/(.*)");
      QRegularExpressionMatch match = ownerRegex.match(rawRepo);
      // Note that the specific values for the error cases below don't matter
      // They are set to avoid rerunning the parsing (since these values won't change mid-run)
      parsedOwner = match.hasMatch() ? match.captured(1) : UnknownOwnerValue();
      parsedRepo = match.hasMatch() ? match.captured(2) : UnknownRepoValue();
    }
    return field == RepoField::owner ? parsedOwner : parsedRepo;
  }
};

#endif // CONSTANTS_H
