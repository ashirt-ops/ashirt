#pragma once

#include <QVariant>
#include <QRegularExpression>
#include <iostream>

#include "helpers/jsonhelpers.h"

namespace dto {
// SemVer is a (mostly) parsed out semantic version (V2). The major, minor, and patch versions are
// converted to ints, while the remainder is left untouched, in "extra".
class SemVer {
 public:
  SemVer(){}
  SemVer(int major, int minor, int patch, QString extra) {
    this->major = major;
    this->minor = minor;
    this->patch = patch;
    this->extra = extra;
  }

  static SemVer parse(QString strTag) {
    QRegularExpression semverRegex(QStringLiteral("^[vV]?(\\d+)\\.(\\d+)\\.(\\d+)(.*)"));
    SemVer ver;

    QRegularExpressionMatch match = semverRegex.match(strTag);

    if (match.hasMatch())
    {
      ver.major = match.captured(1).toInt();
      ver.minor = match.captured(2).toInt();
      ver.patch = match.captured(3).toInt();
      ver.extra = match.captured(4);
    }

    return ver;
  }

  QString toString() {
    return "v" + QString::number(this->major)
                  + "." + QString::number(this->minor)
                  + "." + QString::number(this->patch)
                  + extra;
  }

  static bool isUpgrade(SemVer cur, SemVer next) {
    return isMajorUpgrade(cur, next) ||
           isMinorUpgrade(cur, next) ||
           isPatchUpgrade(cur, next);
  }
  static bool isMajorUpgrade(SemVer cur, SemVer next) {
    return next.major > cur.major;
  }
  static bool isMinorUpgrade(SemVer cur, SemVer next) {
    return next.major == cur.major &&
           next.minor > cur.minor;
  }
  static bool isPatchUpgrade(SemVer cur, SemVer next) {
    return next.major == cur.major &&
           next.minor == cur.minor &&
           next.patch > cur.patch;
  }

  int major = 0;
  int minor = 0;
  int patch = 0;
  QString extra;
};

class GithubRelease {
 public:

  QString url;
  QString htmlURL;
  QString assetsURL;
  QString uploadURL;
  QString tarballURL;
  QString zipballURL;
  QString tagName;
  QString releaseName;
  QString body;
  bool prerelease;
  bool draft;
  QString publishedAt;
  qint64 id;

 public:
  GithubRelease() {
    this->id = 0;
  }

  static GithubRelease parseData(QByteArray data) {
    return parseJSONItem<GithubRelease>(data, GithubRelease::fromJson);
  }

  static std::vector<GithubRelease> parseDataAsList(QByteArray data) {
    return parseJSONList<GithubRelease>(data, GithubRelease::fromJson);
  }

  bool isLegitimate() {
    return this->id != 0;
  }

 private:
  static GithubRelease fromJson(QJsonObject obj) {
    GithubRelease release;
    release.url = obj["url"].toString();
    release.htmlURL = obj["html_url"].toString();
    release.assetsURL = obj["assets_url"].toString();
    release.uploadURL = obj["upload_url"].toString();
    release.tarballURL = obj["tarball_url"].toString();
    release.zipballURL = obj["zipball_url"].toString();
    release.tagName = obj["tag_name"].toString();
    release.body = obj["body"].toString();
    release.releaseName = obj["name"].toString();
    release.publishedAt = obj["published_at"].toString();
    release.draft = obj["draft"].toBool();
    release.prerelease = obj["prerelease"].toBool();

    // missing a number of fields here that maybe aren't relevant

    release.id = obj["id"].toVariant().toLongLong();

    return release;
  }
};

class ReleaseDigest {

 public:
  ReleaseDigest(){}

 public:
  static ReleaseDigest fromReleases(QString curVersion, const std::vector<GithubRelease> &borrowedReleases) {

    if (curVersion.contains(QStringLiteral("v0.0.0"))) {
      std::cerr << "skipping unversioned/development release check" << std::endl;
      return ReleaseDigest();
    }

    std::vector<GithubRelease> upgrades;
    SemVer currentVersion = SemVer::parse(curVersion);

    SemVer majorVer = SemVer(currentVersion);
    SemVer minorVer = SemVer(currentVersion);
    SemVer patchVer = SemVer(currentVersion);

    for (const auto& release : borrowedReleases) {
      if (SemVer::isUpgrade(currentVersion, SemVer::parse(release.tagName))) {
        upgrades.push_back(release);
      }
    }

    auto rtn = ReleaseDigest();

    for (const GithubRelease& upgrade : upgrades) {
      auto upgradeVersion = SemVer::parse(upgrade.tagName);
      if (SemVer::isMajorUpgrade(currentVersion, upgradeVersion) && SemVer::isUpgrade(majorVer, upgradeVersion)) {
        majorVer = upgradeVersion;
        rtn.majorRelease = upgrade;
      }
      else if (SemVer::isMinorUpgrade(currentVersion, upgradeVersion) && SemVer::isUpgrade(minorVer, upgradeVersion)) {
        minorVer = upgradeVersion;
        rtn.minorRelease = upgrade;
      }
      else if (SemVer::isPatchUpgrade(currentVersion, upgradeVersion) && SemVer::isUpgrade(patchVer, upgradeVersion)) {
        patchVer = upgradeVersion;
        rtn.patchRelease = upgrade;
      }
    }

    return rtn;
  }

  bool hasUpgrade() {
    return majorRelease.isLegitimate()
           || minorRelease.isLegitimate()
           || patchRelease.isLegitimate();
  }

 public:
  GithubRelease majorRelease = GithubRelease();
  GithubRelease minorRelease = GithubRelease();
  GithubRelease patchRelease = GithubRelease();

};

}
