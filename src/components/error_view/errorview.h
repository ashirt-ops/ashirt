#pragma once

#include "components/evidencepreview.h"
class QLabel;
/**
 * @brief The ErrorView class provides a default/error handler for situations when an an
 * EvidencePreview is needed, but needs to be displayed with no chance to throw an error. Simply
 * renders the provided error text in a label.
 */
class ErrorView : public EvidencePreview {
  Q_OBJECT
 public:
  explicit ErrorView(QString errorText = QString(), QWidget* parent = nullptr);
  ~ErrorView() = default;
  virtual void loadFromFile(QString filepath) {Q_UNUSED(filepath)}
  virtual void clearPreview() {}
 private:
  QLabel* errorLabel = nullptr;
};
