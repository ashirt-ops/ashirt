// Copyright 2020, Verizon Media
// Licensed under the terms of MIT. See LICENSE file in project root for terms.

#include "credits.h"

#include <QDateTime>
#include <QKeySequence>

#include "helpers/netman.h"
#include "helpers/constants.h"

struct Attribution {
  std::string library;
  std::string libraryUrl;
  std::string authors;
  std::string license;
  std::string licenseUrl;

  Attribution() = default;
  Attribution(std::string library, std::string libraryUrl, std::string authors, std::string license,
              std::string licenseUrl) {
    this->library = library;
    this->libraryUrl = libraryUrl;
    this->authors = authors;
    this->license = license;
    this->licenseUrl = licenseUrl;
  }
};

static std::string hyperlinkMd(std::string label, std::string url) {
  return "[" + label + "](" + url + ")";
}

static std::string attributionMarkdown() {
  std::vector<Attribution> attribs = {
      Attribution("Qt", "http://qt.io", "The Qt Company", "LGPLv3",
                  "https://www.gnu.org/licenses/lgpl-3.0.html"),
      Attribution("QProgressIndicator", "https://github.com/mojocorp/QProgressIndicator",
                  "Mojocorp, et al", "MIT License",
                  "https://github.com/mojocorp/QProgressIndicator/blob/master/LICENSE"),
      Attribution("MultipartEncoder", "https://github.com/AndsonYe/MultipartEncoder", "Ye Yangang",
                  "MIT License",
                  "https://github.com/AndsonYe/MultipartEncoder/blob/master/LICENSE"),
      Attribution("UGlobalHotkey", "https://github.com/joelatdeluxe/UGlobalHotkey",
                  "Anton Konstantinov, et al", "Public Domain", ""),
      Attribution("AspectRatioPixmapLabel", "https://stackoverflow.com/a/22618496/4262552",
                  "phyatt, et al", "CC BY-SA 3.0",
                  "https://creativecommons.org/licenses/by-sa/3.0/"),
  };

  std::string msg =
      "This application uses the following open source software:\n\n"
      "| Project/Library | Authors | License |\n"
      "| --------------- | ------- | ------- |\n";

  for (const auto& attrib : attribs) {
    std::string license;
    std::string suffix;
    if (!attrib.licenseUrl.empty()) {
      license += "[";
      suffix = "](" + attrib.licenseUrl + ")";
    }
    license += attrib.license + suffix;

    // clang-format off
    msg +=  "| " + hyperlinkMd(attrib.library, attrib.libraryUrl) +
           " | " + attrib.authors +
           " | " + license +
           " |\n";
    // clang-format on
  }

  return msg;
}

static std::string copyrightDate() {
  int initialYear = 2020;
  int currentYear = QDateTime::currentDateTime().date().year();
  auto rtn = std::to_string(initialYear);
  if (currentYear != initialYear) {
    rtn += "-" + std::to_string(currentYear);
  }
  return rtn;
}

static std::string preambleMarkdown() {
  const std::string lf = "\n\n";  // double linefeed to add in linebreaks in markdown
  // clang-format off
  return "Version: " + Constants::releaseTag().toStdString() +
         lf + "Commit Hash: " + Constants::commitHash().toStdString() +
         lf + "Copyright " + copyrightDate() + ", Verizon Media" +
         lf + "Licensed under the terms of [MIT](https://github.com/theparanoids/ashirt/blob/master/LICENSE)" +
         lf + "A short user guide can be found " + hyperlinkMd("here", Constants::userGuideUrl().toStdString()) +
         lf + "Report issues " + hyperlinkMd("here", Constants::reportAnIssueUrl().toStdString()) +
         lf;
  // clang-format on
}

static std::string normalBodyMarkdown() {
  // clang-format off
  return "# ASHIRT\n\n"
          + preambleMarkdown()
          + "## Credits\n"
          + attributionMarkdown();
  // clang-format on
}

Credits::Credits(QWidget* parent) : QDialog(parent) {
  buildUi();
  wireUi();
}

void Credits::updateRelease() {
  static QString baseUpdateText = "A new update is available! Click <a href=\"%1\">here</a> for more details.";
  if (updateDigest.hasUpgrade()) {
    updateLabel->setText(baseUpdateText.arg(Constants::releasePageUrl()));
  }
  else {
    updateLabel->clear();
  }
}

void Credits::buildUi() {
  gridLayout = new QGridLayout(this);

  updateLabel = new QLabel();
  updateLabel->setOpenExternalLinks(true);
  updateLabel->setTextFormat(Qt::RichText);
  updateLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);

  creditsArea = new QTextBrowser(this);
  creditsArea->setOpenExternalLinks(true);
  creditsArea->setReadOnly(true);
  creditsArea->setMarkdown(normalBodyMarkdown().c_str());

  buttonBox = new QDialogButtonBox(this);
  buttonBox->addButton(QDialogButtonBox::Close);

  // Layout
  /*                   0
       +------------------------------------+
    0  |         update label               |
       +------------------------------------+
       |                                    |
    1  |       Credits Area                 |
       |                                    |
       +------------------------------------+
    2  | Dialog button Box{close}           |
       +------------------------------------+
  */

  // row 0
  gridLayout->addWidget(updateLabel, 0, 0);

  // row 1
  gridLayout->addWidget(creditsArea, 1, 0);

  // row 2
  gridLayout->addWidget(buttonBox, 2, 0);

  closeWindowAction = new QAction(this);
  closeWindowAction->setShortcut(QKeySequence::Close);
  this->addAction(closeWindowAction);

  this->setLayout(gridLayout);
  this->resize(640, 500);
  this->setWindowTitle("About");

  // Make the dialog pop up above any other windows but retain title bar and buttons
  Qt::WindowFlags flags = this->windowFlags();
  flags |= Qt::CustomizeWindowHint | Qt::WindowStaysOnTopHint | Qt::WindowMinMaxButtonsHint |
           Qt::WindowCloseButtonHint;
  this->setWindowFlags(flags);
}

void Credits::wireUi() {
  connect(closeWindowAction, &QAction::triggered, this, &QDialog::close);
  connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::close);
  connect(&NetMan::getInstance(), &NetMan::releasesChecked, this, &Credits::onReleasesUpdate);
}

Credits::~Credits() {
  delete closeWindowAction;
  delete updateLabel;
  delete creditsArea;
  delete buttonBox;

  delete gridLayout;
}

void Credits::onReleasesUpdate(bool success, std::vector<dto::GithubRelease> releases) {
  if (!success) {
    return; //doesn't matter if this fails
  }

  auto digest = dto::ReleaseDigest::fromReleases(Constants::releaseTag(), releases);
  updateDigest = digest;
  updateRelease();
}
