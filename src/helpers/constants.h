#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <QStandardPaths>
#include <QString>

#include <iostream>
#include <QRegularExpression>

class Constants {
 public:
  static QString unknownRepoValue() {
    return QStringLiteral("???");
  }
  static QString unknownOwnerValue() {
    return QStringLiteral("???");
  }

  static QString configLocation() {
#ifdef Q_OS_MACOS
    return QStringLiteral("%1/config.json").arg(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation));
#else
    return QStringLiteral("%1/ashirt/config.json").arg(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation));
#endif
  }

  static QString dbLocation() {
    return QStringLiteral("%1/evidence.sqlite").arg(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation));
  }

  static QString defaultEvidenceRepo() {
    return QStringLiteral("%1/evidence").arg(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation));
  }

  static QString releaseOwner() {
    return parseRepo(RepoField::owner);
  }

  static QString releaseRepo() {
    return parseRepo(RepoField::repo);
  }

  static QString commitHash() {
    return QStringLiteral("%1").arg(COMMIT_HASH);
  }

  static QString releaseTag() {
    static QString parsedReleaseTag;

    if (parsedReleaseTag.isEmpty()) {
      QRegularExpression compiledTagRegex(QStringLiteral("(?:tags/)?v(.*)"));
      auto rawVersion = QStringLiteral("%1").arg(VERSION_TAG);
      QRegularExpressionMatch match = compiledTagRegex.match(rawVersion);
      parsedReleaseTag = match.hasMatch() ? QStringLiteral("v") + match.captured(1) : QStringLiteral("v0.0.0-unversioned");
    }

    return parsedReleaseTag;
  }

  static QString userGuideUrl() {
    return QStringLiteral("https://www.github.com/theparanoids/ashirt/blob/master/README.md");
  }

  static QString reportAnIssueUrl() {
    return QStringLiteral("https://www.github.com/theparanoids/ashirt/issues");
  }

  static QString releasePageUrl() {
    return QStringLiteral("https://github.com/theparanoids/ashirt/releases");
  }

  static QString codeFont() {
    #ifdef Q_OS_MACX
    return QStringLiteral("monaco");
    #endif

    return QStringLiteral("source code pro");
  }

  /// defaultDbName returns a string storing the "name" of the database for Qt identification
  /// purposes. This _value_ should not be reused for other db connections.
  static QString defaultDbName() {
    return QStringLiteral("evidence");
  }

 private:
  enum RepoField {
    owner = 0,
    repo = 1,
  };

  static QString parseRepo(RepoField field) {
    static QString parsedRepo;
    static QString parsedOwner;

    if (parsedRepo.isEmpty()) {
      auto rawRepo = QStringLiteral("%1").arg(SOURCE_CONTROL_REPO);
      QRegularExpression ownerRegex(QStringLiteral("^([^/]+)/(.*)"));
      QRegularExpressionMatch match = ownerRegex.match(rawRepo);
      // Note that the specific values for the error cases below don't matter
      // They are set to avoid rerunning the parsing (since these values won't change mid-run)
      parsedOwner = match.hasMatch() ? match.captured(1) : unknownOwnerValue();
      parsedRepo = match.hasMatch() ? match.captured(2) : unknownRepoValue();
    }
    return field == RepoField::owner ? parsedOwner : parsedRepo;
  }
};

#endif // CONSTANTS_H
