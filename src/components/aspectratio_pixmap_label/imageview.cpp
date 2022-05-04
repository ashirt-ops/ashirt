#include "imageview.h"

#include <QImageReader>
#include <QPixmap>
#include <QVBoxLayout>

#include "aspectratiopixmaplabel.h"

ImageView::ImageView(QWidget* parent)
  : EvidencePreview(parent)
  , previewImage(new AspectRatioPixmapLabel(this))
{
  buildUi();
}

void ImageView::buildUi() {
  previewImage->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
  previewImage->setAlignment(Qt::AlignCenter);

  auto layout = new QVBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(previewImage);
}

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
