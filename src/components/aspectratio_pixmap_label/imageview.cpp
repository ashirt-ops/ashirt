#include "imageview.h"

#include <QImageReader>
#include <QPixmap>

ImageView::ImageView(QWidget* parent) : EvidencePreview(parent) {
  buildUi();
  wireUi();
}

ImageView::~ImageView() {
  delete previewImage;
  delete gridLayout;
}

void ImageView::buildUi() {
  gridLayout = new QGridLayout(this);
  gridLayout->setContentsMargins(0, 0, 0, 0);

  previewImage = new AspectRatioPixmapLabel(this);
  previewImage->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
  previewImage->setAlignment(Qt::AlignCenter);
  // Layout
  /*        0
       +------------+
    0  |            |
       | Img Label  |
       |            |
       +------------+
  */

  // row 0
  gridLayout->addWidget(previewImage, 0, 0);
}

void ImageView::wireUi() {}

// ---- Parent Overrides

void ImageView::clearPreview() { previewImage->clear(); }

void ImageView::loadFromFile(QString filepath) {
  QImageReader reader(filepath);

  const QImage img = reader.read();
  if (img.isNull()) {
    previewImage->setText(tr("Unable to load preview: %1").arg(reader.errorString()));
  }
  else {
    previewImage->setPixmap(QPixmap::fromImage(img));
  }
}
