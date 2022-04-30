#pragma once

#include <QSqlError>
#include <QSqlQuery>


/**
 * @brief The QueryResult class is a small container for representing a post "query.exec" state
 * for a given query. Note that this class does not *require* that an exec is made. Notably,
 * if a prepared statement is created, and fails, then this could be reflected by this class.
 */
class QueryResult {
 public:
  QueryResult(){}
  QueryResult(QSqlQuery query) {
    this->err = query.lastError();
    this->query = std::move(query);
    this->success = err.type() == QSqlError::NoError;
  }

 public:
  /// success is a shorthand to determine if the last error was actually NoError
  bool success = false;
  /// query is the result of the underlying query, in whatever state it is in
  QSqlQuery query;

  /// err is a shorthand for QSqlQuery.lastError()
  QSqlError err;
};
