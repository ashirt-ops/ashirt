#pragma once

#include "components/evidencepreview.h"

class AspectRatioPixmapLabel;
/**
 * @brief The ImageView class is a thinly wrapped AspectRatioPixmapLabel to meet the EvidencePreview
 * interface requirements.
 */
class ImageView : public EvidencePreview {
  Q_OBJECT
 public:
  explicit ImageView(QWidget* parent = nullptr);
  ~ImageView() = default;

 private:
  /// buildUi constructs the UI, without wiring any connections
  void buildUi();

 public:
  /// loadFromFile attempts to load the indicated image from disk.
  /// If this process fails, renders a text message instead. Inherited from EvidencePreview
  virtual void loadFromFile(QString filepath) override;

  /// clearPreview clears the rendered image. Inherited from EvidencePreview.
  virtual void clearPreview() override;

 private:
  AspectRatioPixmapLabel* previewImage;
};
