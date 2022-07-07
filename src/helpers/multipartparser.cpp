/*********************************************************************************
 *     File Name           :     multipartparser.cpp
 *     Created By          :     Ye Yangang, ReWritten for Qt by Chris Rizzitello
 *     Creation Date       :     [2017-02-20 16:50] Modified for Qt 7/5/2022
 *     Last Modified       :     [AUTO_UPDATE_BEFORE_SAVE]
 *     Description         :     Generate multipart/form-data POST body
 **********************************************************************************/

#include "multipartparser.h"

#include <QFileInfo>

#include "string_helpers.h"

MultipartParser::MultipartParser()
    : m_boundary(QStringLiteral("----ASHIRTTrayApp%1").arg(StringHelpers::randomString(16)))
{ }

const QByteArray &MultipartParser::generateBody()
{
    m_body.clear();
    for (const auto &param : m_paramList) {
        m_body.append(m_contentHeader.arg(m_boundary).toUtf8());
        m_body.append(m_contentParam.arg(param.first).toUtf8());
        m_body.append(param.second.toUtf8());
    }
    for (const auto &pair : m_fileList) {
        QFileInfo info = QFileInfo(pair.second);
        QString name = info.fileName();
        QString ext = info.completeSuffix().toLower();
        QString type = QStringLiteral("application/octet-stream");
        QByteArray data;
        QFile file(pair.second);
        if (file.open(QIODevice::ReadOnly))
            data = file.readAll();
        if(ext.endsWith(QStringLiteral("jpg")) || ext.endsWith(QStringLiteral("jpeg")))
            type = QStringLiteral("image/jpeg");
        else if(ext.endsWith(QStringLiteral("txt")) || ext.endsWith(QStringLiteral("log")))
            type = QStringLiteral("text/plain");
        m_body.append(m_contentHeader.arg(m_boundary).toUtf8());
        m_body.append(m_contentFile.arg(pair.first, name, type).toUtf8());
        m_body.append(data);
    }
    m_body.append(QStringLiteral("\r\n--%1--\r\n").arg(m_boundary).toUtf8());
    return m_body;
}
