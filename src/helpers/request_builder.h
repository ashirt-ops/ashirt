#pragma once

#include <QNetworkAccessManager>
#include <QNetworkReply>

#include "helpers/file_helpers.h"

/// NO_BODY is a shorthand for supplying an empty body contents.
static auto NO_BODY = "";

/// RequestMethod is a small enum representing valid request types.
/// Currently, only GET and POST are supported
enum RequestMethod { METHOD_GET = 0, METHOD_POST };

/// RequestMethodToString converst the given RequestMethod enum into a proper method string.
/// METHOD_GET corresponds to "GET", METHOD_POST corresponds to "POST" and so on
static QString RequestMethodToString(RequestMethod val) {
  static QString names[] = {QStringLiteral("GET"), QStringLiteral("POST")};
  return names[val];
}

/**
 * @brief The RequestBuilder class is a small builder-style class to construct a valid HTTP request.
 *
 * Requests are built via the various mutators (setMethod, setHost, etc). Each mutator returns the
 * modified object, so that requests can be chained together (following the builder pattern). Accessors
 * are provided to inspect set fields (all fields are empty by default). This class provides two
 * ways to "complete" the request building. The `build` function will genrate a QNetworkRequest object
 * while the `execute` function will interpret the set method, and actually submit the request, returning
 * a QNetworkReply*.
 *
 * This class also provides a handful of pre-fab "constructors" to cover common cases.
 *
 * Finally, this class must be cleaned up after it is used. To aid in this, the `execute` terminal
 * supports an "autodelete" flag (defaulting to `true`) that will call `delete this;` just prior
 * to returning the QNetworkReply. If this request needs to persist then simply pass false for autodelete.
 */
class RequestBuilder {
 private:
  RequestMethod method;
  QByteArray body = NO_BODY;
  QString host;
  QString endpoint;

  QString contentType;

  QList<QPair<QString, QString>> rawHeaders;
  QList<QPair<QNetworkRequest::KnownHeaders, QVariant>> knownHeaders;

  // creators (constructors + Psuedo constructors)
 public:
  RequestBuilder() {}
  ~RequestBuilder() {}

  /// newGet constructs a request builder with method GET
  static RequestBuilder* newGet() {
    RequestBuilder* builder = new RequestBuilder();
    return builder->setMethod(METHOD_GET);
  }

  /// newFormPost constructs a request builder with method POST and contentType multipart/form-data
  static RequestBuilder* newFormPost(QString formboundary) {
    RequestBuilder* builder = new RequestBuilder();
    return builder
        ->setMethod(METHOD_POST)
        ->addKnownHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("multipart/form-data; boundary=%1").arg(formboundary));
  }

  /// newJSONPost constructs a request builder with method POST and contentType application/json
  static RequestBuilder* newJSONPost() {
    RequestBuilder* builder = new RequestBuilder();
    return builder
        ->setMethod(METHOD_POST)
        ->addKnownHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
  }

  // accessors
 public:
  /// getHost retrieves the set host
  QString getHost() {
    return this->host;
  }

  /// getBody retrieves the set body
  QByteArray getBody() {
    return this->body;
  }

  /// getEndpoint retrieves the set endpoint
  QString getEndpoint() {
    return this->endpoint;
  }

  /// getMethod retrieves the set method
  RequestMethod getMethod() {
    return this->method;
  }

  // mutators
 public:

  /// addKnownHeader adds a new header to the list of headers. Known headers are simply headers with
  /// a predefined name.
  RequestBuilder* addKnownHeader(QNetworkRequest::KnownHeaders headerName, QVariant value) {
    knownHeaders.append(QPair<QNetworkRequest::KnownHeaders, QVariant>(headerName, value));
    return this;
  }

  /// addRawHeader adds a new header to the list of headers. Raw headers are simply headers without
  /// predefined names.
  RequestBuilder* addRawHeader(QString headerName, QString value) {
    rawHeaders.append(QPair<QString, QString>(headerName, value));
    return this;
  }

  /// setBody sets the body for this request
  RequestBuilder* setBody(QByteArray body) {
    this->body = body;
    return this;
  }

  /// setHost sets the host for this request
  RequestBuilder* setHost(QString host) {
    this->host = host;
    return this;
  }

  /// setEndpoint sets the endpoint for this request
  RequestBuilder* setEndpoint(QString endpoint) {
    this->endpoint = endpoint;
    return this;
  }

  /// setMethod sets the method for this request
  RequestBuilder* setMethod(RequestMethod method) {
    this->method = method;
    return this;
  }

  // finishers
 public:
  /// build completes the request builder by product a QNetworkRequest that can be provided to a
  /// QNetworkAccessManager. Does not clean up the request builder object
  QNetworkRequest build() {
    QNetworkRequest req;

    for(const auto& header : rawHeaders) {
      req.setRawHeader(header.first.toUtf8(), header.second.toUtf8());
    }

    for(const auto& header : knownHeaders) {
      req.setHeader(header.first, header.second);
    }

    QString url = this->host;
    if (url.length() > 0 && url.at(url.size() - 1) == '/') {
      url.chop(1);
    }
    url += endpoint;
    req.setUrl(url);

    return req;
  }

  /// execute builds the network request, then executes based on the given NetworkAccessManager
  /// and set method. In the event that the given method is not supported (by this builder), a nullptr
  /// is returned, and an console error message is printed.
  /// Note: calling this method typically marks the end of the lifecycle of this object. An autodelete
  /// parameter is provided to simplify instance deletion. However, as it is deleted, the caller
  /// must not use the builder after this call, or, must supply autodelete=false to ensure the
  /// instance persists
  QNetworkReply *execute(QNetworkAccessManager* nam, bool autodelete=true) {
    auto req = build();
    QNetworkReply* reply = nullptr;

    switch (method) {
      case METHOD_GET:
        reply = nam->get(req);
        break;
      case METHOD_POST:
        reply = nam->post(req, body);
        break;
      default:
        qWarning() << "Requestbuilder contains an unsupported request method";
    }
    if (autodelete) {
      delete this;
    }
    return reply;
  }
};
