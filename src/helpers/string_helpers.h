#pragma once

#include <QRandomGenerator>
#include <QRegularExpression>
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

  /**
   * @brief isValidOperationSlug Validates an operation slug for safe use in file paths
   * Operation slugs are used in constructing file paths, so they must not contain
   * path traversal sequences or other filesystem-unsafe characters.
   * @param slug The operation slug to validate
   * @return true if slug is safe to use in file paths, false otherwise
   */
  static bool isValidOperationSlug(const QString& slug) {
    if (slug.isEmpty() || slug.length() > 128) {
      return false;
    }
    
    // Only allow alphanumeric, hyphen, underscore
    QRegularExpression validPattern(QStringLiteral("^[a-zA-Z0-9_-]+$"));
    if (!validPattern.match(slug).hasMatch()) {
      return false;
    }
    
    // Explicitly reject path traversal patterns
    if (slug.contains(QStringLiteral("..")) || 
        slug.contains(QStringLiteral("/")) || 
        slug.contains(QStringLiteral("\\"))) {
      return false;
    }
    
    return true;
  }

  private:
    inline static const QString _chars = QStringLiteral("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");
};
