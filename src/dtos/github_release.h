#ifndef GITHUB_RELEASE_H
#define GITHUB_RELEASE_H

#include <QVariant>
#include <QRegularExpression>

#include "helpers/jsonhelpers.h"

namespace dto {
// SemVer is a (mostly) parsed out semantic version (V2). The major, minor, and patch versions are
// converted to ints, while the remainder is left untouched, in "extra".
class SemVer {
 public:
  SemVer() {}
  SemVer(int major, int minor, int patch, QString extra) {
    this->major = major;
    this->minor = minor;
    this->patch = patch;
    this->extra = extra;
  }

  static SemVer parse(QString strTag) {
    QRegularExpression semverRegex("^[vV]?(\\d+)\\.(\\d+)\\.(\\d+)(.*)");
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
    return QString("major: ") + QString::number(major)
           + " minor: " + QString::number(minor)
           + " patch: " + QString::number(patch)
           + " extra: " + extra;
  }

  int major;
  int minor;
  int patch;
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

  static GithubRelease parseData(QByteArray data) {
    return parseJSONItem<GithubRelease>(data, GithubRelease::fromJson);
  }

  static std::vector<GithubRelease> parseDataAsList(QByteArray data) {
    return parseJSONList<GithubRelease>(data, GithubRelease::fromJson);
  }

  std::string toStdString() {

    auto rtn = QString("{ ")
               + "name: " + releaseName + ", "
               + "url:  " + url + ", "
               + "desc: " + body + ", "
               + "pubT: " + publishedAt + ", "
               + "TagN: " + tagName + ", "
               + "Ptag: " + SemVer::parse(tagName).toString() +
               + "}";

    return rtn.toStdString();
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
}

#endif // GITHUB_RELEASE_H
