#pragma once

#include <QRegularExpression>
#include <QStandardPaths>

class Constants {
 public:
  inline static const auto unknownValue = QStringLiteral("???");
  inline static const auto dbLocation = QStringLiteral("%1/evidence.sqlite").arg(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation));
  inline static const auto defaultEvidenceRepo = QStringLiteral("%1/evidence").arg(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation));
  inline static const auto commitHash = QStringLiteral("%1").arg(COMMIT_HASH);
  inline static const auto userGuideUrl = QStringLiteral("https://www.github.com/theparanoids/ashirt/blob/master/README.md");
  inline static const auto reportAnIssueUrl = QStringLiteral("https://www.github.com/theparanoids/ashirt/issues");
  inline static const auto releasePageUrl = QStringLiteral("https://github.com/theparanoids/ashirt/releases");
  /// defaultDbName returns a string storing the "name" of the database for Qt identification
  /// purposes. This _value_ should not be reused for other db connections.
  inline static const auto defaultDbName = QStringLiteral("evidence");
#ifdef Q_OS_MACOS
  inline static const auto configLocation = QStringLiteral("%1/config.json").arg(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation));
  inline static const auto codeFont = QStringLiteral("monaco");
#else
  inline static const auto configLocation = QStringLiteral("%1/ashirt/config.json").arg(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation));
  inline static const auto codeFont = QStringLiteral("source code pro");
#endif

  static QString releaseOwner() {
    return parseRepo(RepoField::owner);
  }

  static QString releaseRepo() {
    return parseRepo(RepoField::repo);
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
      parsedOwner = match.hasMatch() ? match.captured(1) : unknownValue;
      parsedRepo = match.hasMatch() ? match.captured(2) : unknownValue;
    }
    return field == RepoField::owner ? parsedOwner : parsedRepo;
  }
};
