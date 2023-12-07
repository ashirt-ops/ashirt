#include "credits.h"

#include <QDateTime>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QLabel>
#include <QTextBrowser>

#include "helpers/netman.h"

Credits::Credits(QWidget* parent)
  : AShirtDialog(parent, AShirtDialog::commonWindowFlags)
  , updateLabel(new QLabel(this))
{
  setWindowTitle("About");
  connect(NetMan::get(), &NetMan::releasesChecked, this, &Credits::onReleasesUpdate);

  updateLabel->setVisible(false);
  updateLabel->setOpenExternalLinks(true);
  updateLabel->setTextFormat(Qt::RichText);
  updateLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);

  auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Close, this);
  connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::close);

  auto creditsArea = new QTextBrowser(this);
  creditsArea->setOpenExternalLinks(true);
  creditsArea->setReadOnly(true);
  creditsArea->setMarkdown(normalBodyMarkdown);

  auto gridLayout = new QVBoxLayout(this);
  gridLayout->addWidget(updateLabel);
  gridLayout->addWidget(creditsArea);
  gridLayout->addWidget(buttonBox);
  setLayout(gridLayout);
  setMinimumSize(425, 500);
}

void Credits::updateRelease() {
  if (updateDigest.hasUpgrade()) {
    updateLabel->setVisible(true);
    updateLabel->setText(baseUpdateText.arg(ReleaseInfo::releasePageUrl));
  }
  else {
    updateLabel->setVisible(false);
    updateLabel->clear();
  }
}

void Credits::onReleasesUpdate(bool success, QList<dto::GithubRelease> releases) {
  if (!success)
    return;

  auto digest = dto::ReleaseDigest::fromReleases(ReleaseInfo::version, releases);
  updateDigest = digest;
  updateRelease();
}
