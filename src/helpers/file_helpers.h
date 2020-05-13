#ifndef FILE_HELPERS_H
#define FILE_HELPERS_H

#include <QDir>
#include <QFile>
#include <QIODevice>
#include <QRandomGenerator>
#include <QString>
#include <QStringList>
#include <array>

#include "appconfig.h"
#include "appsettings.h"
#include "exceptions/fileerror.h"
#include "helpers/pathseparator.h"

class FileHelpers {
 public:
  /// randomText generates an arbitrary number of case-sensitive english letters
  static QString randomText(unsigned int numChars) {
    std::array<int, 2> asciiOffset = {'A', 'a'};
    QStringList replacement;

    for (unsigned int i = 0; i < numChars; i++) {
      int letter = QRandomGenerator::global()->bounded(52);
      auto base = asciiOffset.at(letter < 26 ? 0 : 1);
      replacement << QString(char(base + (letter % 26)));
    }
    return replacement.join("");
  }

  /**
   * @brief randomFilename replaces a string of 6 consecutive X characters with 6 consecutive random
   * english letters. Each letter may either be upper or lower case. Similar to what QTemporaryFile
   * does.
   * @param templateStr The model string, with
   * @return The resulting filename
   */
  static QString randomFilename(QString templateStr) {
    QString replaceToken = "XXXXXX";
    int templateIndex = templateStr.indexOf(replaceToken);

    QString replacement = randomText(replaceToken.length());
    return templateStr.replace(templateIndex, replaceToken.length(), replacement);
  }

  /// converts a c++ std string into QByteArray, ensuring proper encoding
  static QByteArray stdStringToByteArray(std::string str) {
    return QByteArray(str.c_str(), str.size());
  }

  /**
   * @brief qstringToByteArray converts a QString into a QByteArray, ensuring proper encoding. Only
   * safe for ascii content.
   * @param q The string to convert
   * @return the QString as a QByteArray
   */
  static QByteArray qstringToByteArray(QString q) { return stdStringToByteArray(q.toStdString()); }

  /// Returns (and creates, if necessary) the path to where evidence should be stored (includes
  /// ending path separator)
  static QString pathToEvidence() {
    AppConfig &conf = AppConfig::getInstance();
    auto op = AppSettings::getInstance().operationSlug();
    auto root = conf.evidenceRepo + PATH_SEPARATOR;
    if (op != "") {
      root += op + PATH_SEPARATOR;
    }

    QDir().mkpath(root);
    return root;
  }

  /// writeFile write the provided content to the provided path.
  /// @throws a FileError if there are issues opening or writing to the file.
  static void writeFile(QString path, QString content) {
    writeFile(path, qstringToByteArray(content));
  }

  /// writeFile write the provided content to the provided path.
  /// @throws a FileError if there are issues opening or writing to the file.
  static void writeFile(QString path, QByteArray content) {
    QFile file(path);
    bool opened = file.open(QIODevice::WriteOnly);
    if (opened) {
      file.write(content);
      file.close();
    }
    if (file.error() != QFile::NoError) {
      throw FileError::mkError("Unable to write to file", path.toStdString(), file.error());
    }
  }

  /// readFile reads all of the data from the given path.
  /// @throws a FileError if any issues occur while writing the filethere are issues opening or
  /// reading the file.
  static QByteArray readFile(QString path) {
    QFile file(path);
    QByteArray data;
    bool opened = file.open(QIODevice::ReadOnly);
    if (opened) {
      data = file.readAll();
    }
    if (file.error() != QFile::NoError) {
      throw FileError::mkError("Unable to read from file", path.toStdString(), file.error());
    }
    return data;
  }
};

#endif  // FILE_HELPERS_H
