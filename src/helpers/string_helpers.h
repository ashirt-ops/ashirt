#pragma once

#include <QRandomGenerator>
#include <QString>

class StringHelpers {
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

  private:
    inline static const QString _chars = QStringLiteral("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");
};
