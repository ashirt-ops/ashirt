#ifndef CONFIG_H
#define CONFIG_H

#include <QString>

#include <vector>
#include <functional>

class Config {
 public:
  virtual ~Config() {};

  /// toJson converts the object into a json string (represented as a QByteArray)
  virtual QByteArray toJsonString() = 0;

  // Accessors and Mutators

  /// version returns the version number for this config data
  virtual int version() = 0;

  /// errorText returns back the encounted error's text, if any. By convention, an empty string
  /// indicates no error
  virtual QString errorText() = 0;

  virtual QString evidenceRepo() = 0;
  virtual QString captureScreenAreaCmd() = 0;
  virtual QString captureScreenAreaShortcut() = 0;
  virtual QString captureScreenWindowCmd() = 0;
  virtual QString captureScreenWindowShortcut() = 0;
  virtual QString captureCodeblockShortcut() = 0;

  virtual void setErrorText(QString newVal) = 0;

  virtual void setEvidenceRepo(QString newVal) = 0;
  virtual void setCaptureScreenAreaCmd(QString newVal) = 0;
  virtual void setCaptureScreenAreaShortcut(QString newVal) = 0;
  virtual void setCaptureScreenWindowCmd(QString newVal) = 0;
  virtual void setCaptureScreenWindowShortcut(QString newVal) = 0;
  virtual void setCaptureCodeblockShortcut(QString newVal) = 0;
};

#endif // CONFIG_H
