// Copyright 2020, Verizon Media
// Licensed under the terms of GPLv3. See LICENSE file in project root for terms.

#include "credits.h"
#include "ui_credits.h"

#include <QDateTime>

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
      Attribution("Qt", "http://qt.io", "The Qt Company", "GPL v3",
                  "https://www.gnu.org/licenses/gpl-3.0.html"),
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
static std::string versionData() { return "Beta 1"; }

static std::string userGuideUrl = "https://www.github.com/ascreen/blob/master/README.md";
static std::string reportAnIssueUrl = "https://www.github.com/ascreen/issues";

static std::string preambleMarkdown() {
  const std::string lf = "\n\n";  // double linefeed to add in linebreaks in markdown
  // clang-format off
  return "Version: " + versionData() +
         lf + "Copyright " + copyrightDate() + ", Verizon Media" +
         lf + "Licensed under the terms of [GPLv3](https://www.gnu.org/licenses/gpl-3.0.html)" +
         lf + "A short user guide can be found " + hyperlinkMd("here", userGuideUrl) +
         lf + "Report issues " + hyperlinkMd("here", reportAnIssueUrl) +
         lf;
  // clang-format on
}

static std::string bodyMarkdown() {
  // clang-format off
  return "# AScreen\n\n"
          + preambleMarkdown()
          + "## Credits\n"
          + attributionMarkdown();
  // clang-format on
}

Credits::Credits(QWidget* parent) : QDialog(parent), ui(new Ui::Credits) {
  ui->setupUi(this);

  ui->creditsArea->setMarkdown(bodyMarkdown().c_str());

  // Make the dialog pop up above any other windows but retain title bar and buttons
  Qt::WindowFlags flags = this->windowFlags();
  flags |= Qt::CustomizeWindowHint | Qt::WindowStaysOnTopHint | Qt::WindowMinMaxButtonsHint |
           Qt::WindowCloseButtonHint;
  this->setWindowFlags(flags);
}

Credits::~Credits() { delete ui; }

void Credits::keyPressEvent(QKeyEvent *evt) {
  QDialog::keyPressEvent(evt);
  // Note: Qt::ControlModifier corresponds to Cmd on the mac (meta corresponds to mac control key)
  if( evt->key() == Qt::Key_W && evt->modifiers() == Qt::ControlModifier) {
    close(); // not needed once onCancelClick implements close directly (other branch)
  }
}
