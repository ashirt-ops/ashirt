#pragma once

#include <QString>

/**
 * @brief The Codeblock class represents ASHIRT "codeblock"-style evidence. Codeblocks are
 * represented by a handful of values: the actual content, where the content was retrieved, and what
 * language the codeblock represents.
 *
 * As this file is meant for storage, it also includes the ability to manage its own on-disk file.
 */
class Codeblock {
 public:
  /// default constructor, no data is provided
  Codeblock() = default;
  /// creates a new codeblock with the given content and a new random filename
  /// (all other data must be provided)
  Codeblock(QString content);

  /**
   * @brief readCodeblock parses a local codeblock file and returns back the data as a codeblock
   * @param filepath The path to the codeblock file
   * @return a parsed Codeblock object, ready for use.
   */
  static Codeblock readCodeblock(const QString& filepath);

  static QString mkName();
  static QString extension();
  static QString contentType();

 public:
  /// content stores the actual codeblock data (i.e. the source code)
  QString content;
  /// subtype stores what language the codeblock was written in, or an empty string if plaintext
  QString subtype;
  /// source store where the codeblock was found, typically represented as a url
  QString source;

 private:
  /// filename is the path to where this file was read from/will be written to
  QString filename;

 public:
  /// filePath is a small helper to access the filename
  inline QString filePath() { return filename; }

  /**
   * @brief encode converts the Codeblock into a json-encoded QNyteArray (it's normal
   * representation)
   * @return the encoded Codeblock
   */
  QByteArray encode();
 public:
  /**
   * @brief saveCodeblock encodes the provided codeblock, then writes that codeblock to it's filePath
   * @param codeblock The codeblock to save
   */
  static bool saveCodeblock(Codeblock codeblock);
};
