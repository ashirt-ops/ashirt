#include "captureclipboardpage.h"

#include <QFileDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <appconfig.h>

#include "hotkeymanager.h"
#include "components/custom_keyseq_edit/singlestrokekeysequenceedit.h"

bool CaptureClipboardPage::validatePage()
{
  if(!field("keySequence.clipboard").isValid())
    return false;

  auto keyCombo = QKeySequence::fromString(field("keySequence.clipboard").toString(), QKeySequence::NativeText);
  AppConfig::setValue(CONFIG::SHORTCUT_CAPTURECLIPBOARD, keyCombo.toString(QKeySequence::PortableText));
  HotkeyManager::updateHotkeys();
  return true;
}

void CaptureClipboardPage::initializePage()
{
  HotkeyManager::unregisterKey(HotkeyManager::ACTION_CAPTURE_CLIPBOARD);
  QString sequence = AppConfig::value(CONFIG::SHORTCUT_CAPTURECLIPBOARD);
  setField("keySequence.clipboard", sequence);
  captureClipboardKeySequence->setKeySequence(QKeySequence::fromString(sequence));
}

void CaptureClipboardPage::cleanupPage()
{
  HotkeyManager::updateHotkeys();
  HotkeyManager::unregisterKey(HotkeyManager::ACTION_CAPTURE_WINDOW);
}

CaptureClipboardPage::CaptureClipboardPage(QWidget *parent)
    : WizardPage{WizardPage::Page_CaptureClipboard, parent}
{
  auto f = font();
  auto _lblTitleLabel = new QLabel(this);
  f.setPointSize(titleFont.first);
  f.setWeight(titleFont.second);
  _lblTitleLabel->setFont(f);
  _lblTitleLabel->setText(tr("Capture Cliboard"));

  auto _lblSubtitle = new QLabel(this);
  f.setPointSize(subTitleFont.first);
  f.setWeight(subTitleFont.second);
  _lblSubtitle->setFont(f);
  _lblSubtitle->setWordWrap(true);
  _lblSubtitle->setText(tr("Uploads clipboard images or text to ashirt.\nText is uploaded as a code block"));

  auto _lblB1 = new QLabel(this);
  f.setPointSize(bodyFont.first);
  f.setWeight(bodyFont.second);
  _lblB1->setFont(f);
  _lblB1->setText(tr("\n\n• Enter the shortcut code below"));

  auto _lblBKey = new QLabel(this);
  f.setPointSize(smallFont.first);
  f.setWeight(smallFont.second);
  _lblBKey->setFont(f);
  _lblBKey->setText(tr("Capture Clipboard Shortcut"));

  captureClipboardKeySequence = new SingleStrokeKeySequenceEdit(this);
  captureClipboardKeySequence->setFixedWidth(120);
  registerField("keySequence.clipboard*", captureClipboardKeySequence, "keySequence", SIGNAL(keySequenceChanged(const QKeySequence &)));

  auto t2 = new QVBoxLayout();
  t2->setSpacing(1);
  t2->addWidget(_lblBKey);
  t2->addWidget(captureClipboardKeySequence);

  auto layout = new QVBoxLayout();
  layout->addWidget(_lblTitleLabel);
  layout->addWidget(_lblSubtitle);
  layout->addWidget(_lblB1);
  layout->addSpacerItem(new QSpacerItem(0,60,QSizePolicy::Minimum, QSizePolicy::Fixed));
  layout->addLayout(t2);
  layout->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Minimum, QSizePolicy::Expanding));
  setLayout(layout);
}
