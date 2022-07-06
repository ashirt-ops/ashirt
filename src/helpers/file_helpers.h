#pragma once

#include <QDir>
#include <QFileInfo>
#include <QRandomGenerator>

class FileHelpers {
 public:
  /**
   * @brief randomString Generates a random String of N chars
   * Each character can be a-z either be upper or lower case.
   * @param numberOfChars Length of the string to return default is 6
   * @return The resulting randomString
   */
  static QString randomString(int numberOfChars = 6) {
    QString rString;
    for(int i = 0; i < numberOfChars; i++)
        rString.append(_chars.at(QRandomGenerator::global()->bounded(_chars.length())));
    return rString;
  }

  /// writeFile write the provided content to the provided path.
  /// returns false if failed.
  static bool writeFile(QString fileName, QByteArray content) {
    auto filePath = getDirname(fileName);
    if(!QDir().exists(filePath))
        QDir().mkpath(filePath);
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly))
        return false;
    if (file.write(content) == -1)
        return false;
    return true;
  }

  /// readFile reads all of the data from the given path.
  /// returns QByteArray() if unable to read.
  /// writes any Errors while reading
  /// reading the file.
  static QByteArray readFile(QString path) {
    QFile file(path);
    QByteArray data;
    if (file.open(QIODevice::ReadOnly))
        data = file.readAll();
    if (file.error() != QFile::NoError)
      QTextStream(stderr) << "Unable to read from file: " << path << '\n' << file.error();
    return data;
  }

  /// getDirname is a small helper to convert a filepath to a file into a path to the file's parent
  static QString getDirname(QString filepath) { return QFileInfo(filepath).dir().path(); }

  private:
    inline static const QString _chars = QStringLiteral("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");
};
