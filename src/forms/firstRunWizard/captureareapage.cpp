#include "captureareapage.h"

#include <QFileDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <appconfig.h>

#include "hotkeymanager.h"
#include "components/custom_keyseq_edit/singlestrokekeysequenceedit.h"

bool CaptureAreaPage::validatePage()
{
  if(!field("command.area").isValid())
    return false;
  AppConfig::setValue(CONFIG::COMMAND_SCREENSHOT, field("command.area").toString());

  if(!field("keySequence.area").isValid())
    return false;
  auto keyCombo = QKeySequence::fromString(field("keySequence.area").toString(), QKeySequence::NativeText);
  AppConfig::setValue(CONFIG::SHORTCUT_SCREENSHOT, keyCombo.toString(QKeySequence::PortableText));
  HotkeyManager::updateHotkeys();
  return true;
}

void CaptureAreaPage::initializePage()
{
  HotkeyManager::unregisterKey(HotkeyManager::ACTION_CAPTURE_AREA);
  QString captureAreaCommand = AppConfig::value(CONFIG::COMMAND_SCREENSHOT);
  if(!captureAreaCommand.isEmpty()) {
    setField("command.area", captureAreaCommand);
    captureAreaLine->setText(captureAreaCommand);
  }

  QString sequence = AppConfig::value(CONFIG::SHORTCUT_SCREENSHOT);
  if(!sequence.isEmpty()) {
    setField("keySequence.area", sequence);
    captureAreaKeySequenceEdit->setKeySequence(QKeySequence::fromString(sequence));
  }
}

void CaptureAreaPage::cleanupPage()
{
  HotkeyManager::updateHotkeys();
}

CaptureAreaPage::CaptureAreaPage(QWidget *parent)
    : WizardPage{Page_CaptureArea, parent}
{
  auto f = font();
  auto _lblTitleLabel = new QLabel(this);
  f.setPointSize(titleFont.first);
  f.setWeight(titleFont.second);
  _lblTitleLabel->setFont(f);
  _lblTitleLabel->setText(tr("Capture Area"));

  auto _lblSubtitle = new QLabel(this);
  f.setPointSize(subTitleFont.first);
  f.setWeight(subTitleFont.second);
  _lblSubtitle->setFont(f);
  _lblSubtitle->setWordWrap(true);
  _lblSubtitle->setText(tr("Set the capture area command and shortcut."));

  auto _lblB1 = new QLabel(this);
  f.setPointSize(bodyFont.first);
  f.setWeight(bodyFont.second);
  _lblB1->setFont(f);
  _lblB1->setText(tr("<html><br><br>• Enter the command ashirt will use to capture area screenshots<br><br>• Enter a key combination that will trigger the capture area command<html>"));

  auto hLayout = new QHBoxLayout();
  hLayout->addWidget(_lblB1, 0, Qt::AlignHCenter);

  auto _lblAKey = new QLabel(this);
  f.setPointSize(smallFont.first);
  f.setWeight(smallFont.second);
  _lblAKey->setFont(f);
  _lblAKey->setText(tr("Capture Area Command"));

  captureAreaLine = new QLineEdit(this);
  captureAreaLine->setTextMargins(3,0,3,0);
  registerField("command.area*", captureAreaLine);

  auto t1 = new QVBoxLayout();
  t1->setSpacing(1);
  t1->addWidget(_lblAKey);
  t1->addWidget(captureAreaLine);

  auto _lblBKey = new QLabel(this);
  f.setPointSize(smallFont.first);
  f.setWeight(smallFont.second);
  _lblBKey->setFont(f);
  _lblBKey->setText(tr("Shortcut"));

  captureAreaKeySequenceEdit = new SingleStrokeKeySequenceEdit(this);
  registerField("keySequence.area*", captureAreaKeySequenceEdit, "keySequence", SIGNAL(keySequenceChanged(const QKeySequence &)));

  auto t2 = new QVBoxLayout();
  t2->setSpacing(1);
  t2->addWidget(_lblBKey);
  t2->addWidget(captureAreaKeySequenceEdit);

  auto tLayout = new QHBoxLayout();
  tLayout->addLayout(t1, 8);
  tLayout->addLayout(t2, 2);

  auto layout = new QVBoxLayout();
  layout->addWidget(_lblTitleLabel);
  layout->addWidget(_lblSubtitle);
  layout->addLayout(hLayout);
  layout->addSpacerItem(new QSpacerItem(0,60,QSizePolicy::Minimum, QSizePolicy::Fixed));
  layout->addLayout(tLayout);
  layout->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Minimum, QSizePolicy::Expanding));
  setLayout(layout);
}
