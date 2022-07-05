/*********************************************************************************
 *     File Name           :     multipartparser.h
 *     Created By          :     Ye Yangang, ReWritten for Qt by Chris Rizzitello
 *     Creation Date       :     [2017-02-20 16:50] Modified for Qt 7/5/2022
 *     Last Modified       :     [AUTO_UPDATE_BEFORE_SAVE]
 *     Description         :     Generate multipart/form-data POST body
 **********************************************************************************/

#pragma once

#include <QList>
#include <QPair>
#include <QString>

class MultipartParser {
 public:
  MultipartParser();
  inline const QString &boundary() {return m_boundary;}
  inline void addParameter(const QString &name = QString(), const QString &value = QString()) {
      m_paramList.append(QPair<QString, QString>(name, value));
  }
  inline void addFile(const QString &name = QString(), const QString &value = QString()) {
      m_fileList.append(QPair<QString, QString>(name, value));
  }
  const QByteArray &generateBody();
 private:
  inline static const auto m_contentHeader = QStringLiteral("\r\n--%1\r\n");
  inline static const auto m_contentParam = QStringLiteral("Content-Disposition: form-data; name=\"%1\"\r\n\r\n");
  inline static const auto m_contentFile = QStringLiteral("Content-Disposition: form-data; name=\"%1\"; filename=\"%2\"\r\nContent-Type: %3\r\n\r\n");
  QString m_boundary;
  QByteArray m_body;
  QList<QPair<QString, QString>> m_paramList;
  QList<QPair<QString, QString>> m_fileList;
};
