#pragma once

#include <QMessageAuthenticationCode>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <algorithm>

#include "appconfig.h"
#include "request_builder.h"
#include "dtos/operation.h"
#include "dtos/tag.h"
#include "dtos/github_release.h"
#include "dtos/checkConnection.h"
#include "helpers/multipartparser.h"
#include "helpers/cleanupreply.h"
#include "helpers/http_status.h"
#include "models/evidence.h"

class NetMan : public QObject {
  Q_OBJECT
public:
  //Result for Connection Test
  enum TestResult {
   INPROGRESS,
   SUCCESS,
   FAILURE
  };

 // type alias QList<dto::Operation> to provide shorter lines
  using OperationVector = QList<dto::Operation>;
  static NetMan* get() {
    static NetMan i;
    return &i;
  }

  /// uploadAsset takes the given Evidence model, encodes it (and the file), and uploads this
  /// to the configured ASHIRT API server. Returns a QNetworkReply to track the request
  /// Note: does not specify the occurred_at field, so occurred_at will reflect the time of upload,
  /// rather than the time of capture.
  static QNetworkReply* uploadAsset(model::Evidence evidence) {
    MultipartParser parser;
    parser.addParameter(QStringLiteral("notes"), evidence.description);
    parser.addParameter(QStringLiteral("contentType"), evidence.contentType);
    // TODO: convert this time below into a proper unix timestamp (mSecSinceEpoch and secsSinceEpoch
    // produce invalid times)
    // parser.AddParameter("occurred_at", std::to_string(evidence.recordedDate);
    QStringList list;
    for (const auto& tag : evidence.tags)
        list.append(QString::number(tag.serverTagId));

    parser.addParameter(QStringLiteral("tagIds"), QStringLiteral("[%1]").arg(list.join(QStringLiteral(","))));
    parser.addFile(QStringLiteral("file"), evidence.path);
    auto builder = ashirtFormPost(QStringLiteral("/api/operations/%1/evidence").arg(evidence.operationSlug), parser.generateBody(), parser.boundary());
    addASHIRTAuth(builder);
    return builder->execute(get()->nam);
  }

  ///Return the last Test Error
  static QString lastTestError() {return get()->_lastTestError;}

  ///Processes the test result and emits sets the lastError and a emits a result.
  static void processTestResults() {
    bool ok = true;
    auto statusCode = get()->testConnectionReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(&ok);

    if(!ok) {
        get()->_lastTestError = tr("Server not Found, Check the Url");
        Q_EMIT get()->testStatusChanged(FAILURE);
        cleanUpReply(&get()->testConnectionReply);
        return;
    }

    dto::CheckConnection connectionCheckResp;
    switch (statusCode) {
        case HttpStatus::StatusOK:
          connectionCheckResp = dto::CheckConnection::parseJson(get()->testConnectionReply->readAll());
          if (connectionCheckResp.parsedCorrectly && connectionCheckResp.ok) {
            get()->_lastTestError = tr("Successfully Connected");
            Q_EMIT get()->testStatusChanged(SUCCESS);
          } else {
            get()->_lastTestError = tr("Server Error Report to Admin");
            Q_EMIT get()->testStatusChanged(TestResult::FAILURE);
          }
          break;
        case HttpStatus::StatusUnauthorized:
          get()->_lastTestError = tr("Authorization Failure, Check Api Keys");
          Q_EMIT get()->testStatusChanged(TestResult::FAILURE);
          break;
        default:
          get()->_lastTestError = tr("Code %1").arg(statusCode);
          Q_EMIT get()->testStatusChanged(TestResult::FAILURE);
    }
    cleanUpReply(&get()->testConnectionReply);
  }

  /// testConnection provides a mechanism to validate a given host, apikey and secret key, to test
  /// a connection to the ASHIRT API server
  /// Connect to the testStatusChanged signal to see results;
  static void testConnection(QString host, QString apiKey, QString secretKey) {
    auto builder = ashirtGet(QStringLiteral("/api/checkconnection"), host);
    addASHIRTAuth(builder, apiKey, secretKey);
    get()->testConnectionReply = builder->execute(get()->nam);
    get()->_lastTestError = tr("Testing in progress");
    Q_EMIT get()->testStatusChanged(TestResult::INPROGRESS);
    connect(get()->testConnectionReply, &QNetworkReply::finished, get(), processTestResults);
  }

  /// getAllOperations retrieves all (user-visble) operations from the configured ASHIRT API server.
  /// Note: normally you should opt to use refreshOperationsList and retrieve the results by listening
  /// for the operationListUpdated signal.
  static QNetworkReply *getAllOperations() {
    auto builder = ashirtGet(QStringLiteral("/api/operations"));
    addASHIRTAuth(builder);
    return builder->execute(get()->nam);
  }

  /// getGithubReleases retrieves the recent releases from github for the provided owner and repo.
  /// Note that normally you should call checkForNewRelease
  static QNetworkReply *getGithubReleases(QString owner, QString repo) {
    return RequestBuilder::newGet()
        ->setHost(QStringLiteral("https://api.github.com"))
        ->setEndpoint(QStringLiteral("/repos/%1/%2/releases").arg(owner, repo))
        ->execute(get()->nam);
  }

  /// refreshOperationsList retrieves the operations currently visible to the user. Results should be
  /// retrieved by listening for the operationListUpdated signal
  static void refreshOperationsList() {
    if (get()->allOpsReply)
        return;
    get()->allOpsReply = get()->getAllOperations();
    connect(get()->allOpsReply, &QNetworkReply::finished, get(), &NetMan::onGetOpsComplete);
  }

  /// getOperationTags retrieves the tags for specified operation from the ASHIRT API server
  static QNetworkReply *getOperationTags(QString operationSlug) {
    auto builder = ashirtGet(QStringLiteral("/api/operations/%1/tags").arg(operationSlug));
    addASHIRTAuth(builder);
    return builder->execute(get()->nam);
  }

  /// createTag attempts to create a new tag for specified operation from the ASHIRT API server.
  static QNetworkReply *createTag(dto::Tag tag, QString operationSlug) {
    auto builder = ashirtJSONPost(QStringLiteral("/api/operations/%1/tags").arg(operationSlug), dto::Tag::toJson(tag));
    addASHIRTAuth(builder);
    return builder->execute(get()->nam);
  }

  /// createOperation attempts to create a new operation with the given name and slug
  static QNetworkReply *createOperation(QString name, QString slug) {
    auto builder = ashirtJSONPost(QStringLiteral("/api/operations"), dto::Operation::createOperationJson(name, slug));
    addASHIRTAuth(builder);
    return builder->execute(get()->nam);
  }

  /// extractResponse inspects the provided QNetworkReply and returns back the contents of the reply.
  /// In addition, it will also indicated, via the provided valid flag, if the response was valid.
  /// A Valid response is one that has a 200 or 201 response AND had no errors flaged from Qt
  /// Note: this applies to ASHIRT responses only
  static QByteArray extractResponse(QNetworkReply *reply, bool &valid) {
    auto status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
    auto reqErr = reply->error();
    valid = (QNetworkReply::NoError == reqErr && status.isValid());
    valid = valid && (status == 200 || status == 201);
    return reply->readAll();
  }

  /// checkForNewRelease retrieves the recent releases from github for the provided owner/repo project.
  /// Callers should retrieve the result by listening for the releasesChecked signal
  static void checkForNewRelease(QString owner, QString repo) {
    if (owner.isEmpty() || repo.isEmpty()) {
      qInfo() << "Skipping release check: no owner or repo set.";
      return;
    }
    get()->githubReleaseReply = get()->getGithubReleases(owner, repo);
    connect(get()->githubReleaseReply, &QNetworkReply::finished, get(), &NetMan::onGithubReleasesComplete);
  }

signals:
 void operationListUpdated(bool success, NetMan::OperationVector  operations = NetMan::OperationVector());
 void releasesChecked(bool success, QList<dto::GithubRelease> releases = QList<dto::GithubRelease>());
 void testStatusChanged(int newStatus);

private:
 NetMan(QObject * parent = nullptr) : QObject(parent), nam(new QNetworkAccessManager(this)) { }
 NetMan(NetMan const &) = delete;
 void operator=(NetMan const &) = delete;

 ~NetMan() {
    cleanUpReply(&get()->allOpsReply);
    cleanUpReply(&get()->testConnectionReply);
    cleanUpReply(&get()->githubReleaseReply);
 };

 QString _lastTestError;

 /// ashirtGet generates a basic GET request to the ashirt API server. No authentication is
 /// provided (use addASHIRTAuth to do this)
 /// Allows for an optional altHost parameter, in order to check for ashirt servers.
 /// Normal usage should provide no value for this parameter.
 static RequestBuilder* ashirtGet(QString endpoint, const QString & altHost= QString()) {
   QString base = (altHost.isEmpty()) ? AppConfig::value(CONFIG::APIURL) : altHost;
   return RequestBuilder::newGet()
       ->setHost(base)
       ->setEndpoint(endpoint);
 }

 /// ashirtJSONPost generates a basic POST request with content type application/json. No
 /// authentication is provided (use addASHIRTAuth to do this)
 static RequestBuilder* ashirtJSONPost(QString endpoint, QByteArray body) {
   return RequestBuilder::newJSONPost()
       ->setHost(AppConfig::value(CONFIG::APIURL))
       ->setEndpoint(endpoint)
       ->setBody(body);
 }

 /// ashirtFormPost generates a basic POST request with content type multipart/form-data.
 /// No authentication is provided (use addASHIRTAuth to do this)
 static RequestBuilder* ashirtFormPost(QString endpoint, QByteArray body, QString boundry) {
   return RequestBuilder::newFormPost(boundry)
       ->setHost(AppConfig::value(CONFIG::APIURL))
       ->setEndpoint(endpoint)
       ->setBody(body);
 }

 /// addASHIRTAuth takes the provided RequestBuilder and adds on Authorization and Date headers
 /// in order to properly authenticate with ASHIRT servers. Note that this should not be used for
 /// non-ashirt requests
 static void addASHIRTAuth(RequestBuilder* reqBuilder, const QString& altApiKey = QString(),
                    const QString& altSecretKey = QString()) {
   auto now = QDateTime::currentDateTimeUtc().toString(QStringLiteral("ddd, dd MMM yyyy hh:mm:ss 'GMT'"));
   reqBuilder->addRawHeader(QStringLiteral("Date"), now);

   // load default key if not present
   QString apiKeyCopy = altApiKey.isEmpty() ? AppConfig::value(CONFIG::ACCESSKEY) : QString(altApiKey);

   auto code = generateHash(RequestMethodToString(reqBuilder->getMethod()),
                            reqBuilder->getEndpoint(), now, reqBuilder->getBody(), altSecretKey);

   auto authValue = QStringLiteral("%1:%2").arg(apiKeyCopy, code);
   reqBuilder->addRawHeader(QStringLiteral("Authorization"), authValue);
 }


 /// generateHash provides a cryptographic hash for ASHIRT api server communication
 static QString generateHash(QString method, QString path, QString date, QByteArray body = NO_BODY,
                      const QString &secretKey = QString()) {

   QString msg  = QStringLiteral("%1\n%2\n%3\n").arg(method, path, date);
   QString secretKeyCopy = secretKey.isEmpty() ? AppConfig::value(CONFIG::SECRETKEY) : QString(secretKey);

   QMessageAuthenticationCode code(QCryptographicHash::Sha256);
   code.setKey(QByteArray::fromBase64(secretKeyCopy.toUtf8()));
   code.addData(msg.toLatin1());
   code.addData(QCryptographicHash::hash(body, QCryptographicHash::Sha256));
   return code.result().toBase64();
 }

 /// onGetOpsComplete is called when the network request associated with the method refreshOperationsList
 /// completes. This will emit an operationListUpdated signal.
 static void onGetOpsComplete() {
   bool isValid;
   auto data = get()->extractResponse(get()->allOpsReply, isValid);
   if (isValid) {
     OperationVector ops = dto::Operation::parseDataAsList(data);
     std::sort(ops.begin(), ops.end(),
               [](dto::Operation i, dto::Operation j) { return i.name < j.name; });

     Q_EMIT get()->operationListUpdated(true, ops);
   } else {
     Q_EMIT get()->operationListUpdated(false);
   }
   cleanUpReply(&get()->allOpsReply);
 }

 /// onGithubReleasesComplete is called when the network request associated with the method checkForNewRelease
 /// completes. This will emit a releasesChecked signal
 static void onGithubReleasesComplete() {
   bool isValid;
   auto data = get()->extractResponse(get()->githubReleaseReply, isValid);
   if (isValid) {
     auto releases = dto::GithubRelease::parseDataAsList(data);
     Q_EMIT get()->releasesChecked(true, releases);
   } else {
     Q_EMIT get()->releasesChecked(false);
   }
   cleanUpReply(&get()->githubReleaseReply);
 }
 QNetworkReply *allOpsReply = nullptr;
 QNetworkReply *testConnectionReply = nullptr;
 QNetworkReply *githubReleaseReply = nullptr;
 QNetworkAccessManager *nam = nullptr;
};
