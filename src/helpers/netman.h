// Copyright 2020, Verizon Media
// Licensed under the terms of MIT. See LICENSE file in project root for terms.

#ifndef NETMAN_H
#define NETMAN_H

#include <QMessageAuthenticationCode>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

#include "appconfig.h"
#include "dtos/operation.h"
#include "dtos/tag.h"
#include "helpers/file_helpers.h"
#include "helpers/multipartparser.h"
#include "helpers/stopreply.h"
#include "models/evidence.h"

static auto NO_BODY = "";

class NetMan : public QObject {
  Q_OBJECT

 public:
  static NetMan &getInstance() {
    static NetMan instance;
    return instance;
  }
  NetMan(NetMan const &) = delete;
  void operator=(NetMan const &) = delete;

  enum RequestMethod { METHOD_GET = 0, METHOD_POST };
  static QString RequestMethodToString(RequestMethod val) {
    static QString names[] = {"GET", "POST"};
    return names[val];
  }

 signals:

  void operationListUpdated(bool success,
                            std::vector<dto::Operation> operations = std::vector<dto::Operation>());
  void testConnectionComplete(bool connected, int statusCode);

 private:
  QNetworkAccessManager *nam;

  NetMan() { nam = new QNetworkAccessManager; }
  ~NetMan() {
    delete nam;
    stopReply(&allOpsReply);
  }

  // mkApiUrl creates a new URL with the appropriate request start
  QString mkApiUrl(QString endpoint, QString host = "") {
    QString base = (host == "") ? AppConfig::getInstance().apiURL : host;
    if (base.size() == 0) {  // if a user hasn't set up the application, then base could be empty
      return endpoint;
    }
    if (base.at(base.size() - 1) == '/') {
      base.chop(1);
    }
    return base + endpoint;
  }

  QString getRFC1123Date() {
    return QDateTime::currentDateTimeUtc().toString("ddd, dd MMM yyyy hh:mm:ss 'GMT'");
  }

  QString generateHash(QString method, QString path, QString date, QByteArray body = NO_BODY,
                       const QString &secretKey = "") {
    auto hashedBody = QCryptographicHash::hash(body, QCryptographicHash::Sha256);
    std::string msg = (method + "\n" + path + "\n" + date + "\n").toStdString();
    msg += hashedBody.toStdString();

    QString secretKeyCopy = QString(secretKey);
    if (secretKeyCopy.isEmpty()) {
      secretKeyCopy = AppConfig::getInstance().secretKey;
    }

    QMessageAuthenticationCode code(QCryptographicHash::Sha256);
    QByteArray key = QByteArray::fromBase64(FileHelpers::qstringToByteArray(secretKeyCopy));

    code.setKey(key);
    code.addData(FileHelpers::stdStringToByteArray(msg));
    return code.result().toBase64();
  }

  QNetworkReply *makeJsonRequest(RequestMethod method, QString endpoint, QByteArray body = NO_BODY,
                                 const QString &host = "", const QString &apiKey = "",
                                 const QString &secretKey = "") {
    QNetworkRequest req = prepRequest(method, endpoint, body, host, apiKey, secretKey);

    switch (method) {
      case METHOD_GET:
        return nam->get(req);
      case METHOD_POST:
        req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        return nam->post(req, body);
      default:
        std::cerr << "makeRequest passed an unsupported method" << std::endl;
    }
    return nullptr;
  }

  QNetworkReply *makeFormRequest(RequestMethod method, QString endpoint, QString boundry,
                                 QByteArray body = NO_BODY, QString host = "") {
    QNetworkRequest req = prepRequest(method, endpoint, body, host);

    switch (method) {
      case METHOD_GET:
        return nam->get(req);
      case METHOD_POST:
        req.setHeader(QNetworkRequest::ContentTypeHeader,
                      "multipart/form-data; boundary=" + boundry);
        return nam->post(req, body);
      default:
        std::cerr << "makeRequest passed an unsupported method" << std::endl;
    }
    return nullptr;
  }

  QNetworkRequest prepRequest(RequestMethod method, QString endpoint, QByteArray body, QString host,
                              const QString &apiKey = "", const QString &secretKey = "") {
    QNetworkRequest req;
    auto now = getRFC1123Date();
    QString reqMethod = RequestMethodToString(method);

    QString apiKeyCopy = QString(apiKey);
    if (apiKeyCopy.isEmpty()) {
      apiKeyCopy = AppConfig::getInstance().accessKey;
    }

    auto code = generateHash(reqMethod, endpoint, now, body, secretKey);
    auto authValue = apiKeyCopy + ":" + code;

    req.setUrl(mkApiUrl(endpoint, host));
    req.setRawHeader(QByteArray("Date"), FileHelpers::qstringToByteArray(now));
    req.setRawHeader(QByteArray("Authorization"), FileHelpers::qstringToByteArray(authValue));

    return req;
  }

  void onGetOpsComplete() {
    bool isValid;
    auto data = extractResponse(allOpsReply, isValid);
    if (isValid) {
      std::vector<dto::Operation> ops = dto::Operation::parseDataAsList(data);
      std::sort(ops.begin(), ops.end(),
                [](dto::Operation i, dto::Operation j) { return i.name < j.name; });

      emit operationListUpdated(true, ops);
    }
    else {
      emit operationListUpdated(false);
    }
    tidyReply(&allOpsReply);
  }

 public:
  QNetworkReply *uploadAsset(model::Evidence evidence) {
    MultipartParser parser;
    parser.AddParameter("notes", evidence.description.toStdString());
    parser.AddParameter("contentType", evidence.contentType.toStdString());

    // TODO: convert this time below into a proper unix timestamp (mSecSinceEpoch and secsSinceEpoch
    // produce invalid times)
    // parser.AddParameter("occurred_at", std::to_string(evidence.recordedDate);

    QStringList list;
    for (auto tag : evidence.tags) {
      list << QString::number(tag.serverTagId);
    }
    parser.AddParameter("tagIds", ("[" + list.join(",") + "]").toStdString());

    parser.AddFile("file", evidence.path.toStdString());

    auto body = FileHelpers::stdStringToByteArray(parser.GenBodyContent());
    return makeFormRequest(METHOD_POST, "/api/operations/" + evidence.operationSlug + "/evidence",
                           parser.boundary().c_str(), body);
  }

  QNetworkReply *testConnection(QString host, QString apiKey, QString secretKey) {
    return makeJsonRequest(METHOD_GET, "/api/checkconnection", NO_BODY, host, apiKey, secretKey);
  }

  QNetworkReply *getAllOperations() { return makeJsonRequest(METHOD_GET, "/api/operations"); }

  void refreshOperationsList() {
    allOpsReply = getAllOperations();
    connect(allOpsReply, &QNetworkReply::finished, this, &NetMan::onGetOpsComplete);
  }

  QNetworkReply *getOperationTags(QString operationSlug) {
    auto url = "/api/operations/" + operationSlug + "/tags";
    return makeJsonRequest(METHOD_GET, url);
  }

  QNetworkReply *createTag(dto::Tag tag, QString operationSlug) {
    auto url = "/api/operations/" + operationSlug + "/tags";
    return makeJsonRequest(METHOD_POST, url, dto::Tag::toJson(tag));
  }

  static QByteArray extractResponse(QNetworkReply *reply, bool &valid) {
    auto status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
    auto reqErr = reply->error();
    valid = (QNetworkReply::NoError == reqErr && status.isValid());
    valid = valid && (status == 200 || status == 201);
    return reply->readAll();
  }

 private:
  QNetworkReply *allOpsReply = nullptr;
};

#endif  // NETMAN_H
