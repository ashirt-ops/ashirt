#ifndef ERRORVIEW_H
#define ERRORVIEW_H

#include <QGridLayout>
#include <QLabel>
#include <QString>
#include <QWidget>

#include "components/evidencepreview.h"

/**
 * @brief The ErrorView class provides a default/error handler for situations when an an
 * EvidencePreview is needed, but needs to be displayed with no chance to throw an error. Simply
 * renders the provided error text in a label.
 */
class ErrorView : public EvidencePreview {
  Q_OBJECT
 public:
  explicit ErrorView(QString errorText, QWidget* parent = nullptr);
  ~ErrorView();

 private:
  /// buildUi constructs the UI, without wiring any connections
  void buildUi();

  /// wireUi connects UI elements together (currently a no-op)
  void wireUi();

 public:
  /// loadFromFile is a no-op. No files are loaded. Inherited from EvidencePreview
  virtual void loadFromFile(QString filepath) override;

  /// clearPreview is a no-op. The initial text is always displayed. Inherited from EvidencePreview.
  virtual void clearPreview() override;

 private:
  QGridLayout* gridLayout;
  QLabel* errorLabel;

  QString errorText;
};

#endif  // ERRORVIEW_H
