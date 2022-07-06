#pragma once

#include <QWidget>

/**
 * @brief The EvidencePreview class is a (non-pure) virtual class that provides a thin wrapper
 * around individual evidence previews. This ensures some common, basic functionality across
 * evidence types.
 */
class EvidencePreview : public QWidget {
  Q_OBJECT
 public:
  explicit EvidencePreview(QWidget *parent = nullptr);

 public:
  /**
   * @brief loadFromFile is a pure virtual method allowing each preview to load its data from disk
   * (where all evidence types live)
   * @param filepath is the full path to the evidence file
   */
  virtual void loadFromFile(QString filepath) = 0;

  /// clearPreview is a pure virtual method that requests the preview content to render a
  /// default/plain view. No content should be displayed.
  virtual void clearPreview() = 0;

  /// saveEvidence allows the underlying evidence to be re-written to disk.
  virtual bool saveEvidence();

  /// setReadonly marks the evidence preview as read-only, disallowing editing. The default
  /// implementation sets the internal flag -- it is the responsibilty of the underlying evidence to
  /// act on this data.
  virtual void setReadonly(bool readonly);

  /// isReadOnly returns whether the current preview has been marked as readonly.
  inline bool isReadOnly() { return readonly; }

 private:
  bool readonly = false;
};
