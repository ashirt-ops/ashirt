#include "capturewindowpage.h"

#include <QFileDialog>
#include <QKeySequenceEdit>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <appconfig.h>

#include "hotkeymanager.h"
#include "components/custom_keyseq_edit/singlestrokekeysequenceedit.h"

bool CaptureWindowPage::validatePage()
{
  if(!field("command.window").isValid())
    return false;
  AppConfig::setValue(CONFIG::COMMAND_CAPTUREWINDOW, field("command.window").toString());

  if(!field("keySequence.window").isValid())
    return false;
  auto keyCombo = QKeySequence::fromString(field("keySequence.window").toString(), QKeySequence::NativeText);
  AppConfig::setValue(CONFIG::SHORTCUT_CAPTUREWINDOW, keyCombo.toString(QKeySequence::PortableText));
  HotkeyManager::updateHotkeys();
  return true;
}

void CaptureWindowPage::initializePage()
{
  HotkeyManager::unregisterKey(HotkeyManager::ACTION_CAPTURE_WINDOW);
  QString captureWindowCommand = AppConfig::value(CONFIG::COMMAND_CAPTUREWINDOW);
  setField("command.window", captureWindowCommand);
  captureWindowLine->setText(captureWindowCommand);

  QString sequence = AppConfig::value(CONFIG::SHORTCUT_CAPTUREWINDOW);
  setField("keySequence.window", sequence);
  captureWindowKeySequence->setKeySequence(QKeySequence::fromString(sequence));
}

void CaptureWindowPage::cleanupPage()
{
  HotkeyManager::updateHotkeys();
  HotkeyManager::unregisterKey(HotkeyManager::ACTION_CAPTURE_AREA);
}

CaptureWindowPage::CaptureWindowPage(QWidget *parent)
    : WizardPage{Page_CaptureWindow, parent}
{
  auto f = font();
  auto _lblTitleLabel = new QLabel(this);
  f.setPointSize(titleFont.first);
  f.setWeight(titleFont.second);
  _lblTitleLabel->setFont(f);
  _lblTitleLabel->setText(tr("Capture Window"));

  auto _lblSubtitle = new QLabel(this);
  f.setPointSize(subTitleFont.first);
  f.setWeight(subTitleFont.second);
  _lblSubtitle->setFont(f);
  _lblSubtitle->setWordWrap(true);
  _lblSubtitle->setText(tr("Set the capture window command and shortcut."));

  auto _lblB1 = new QLabel(this);
  f.setPointSize(bodyFont.first);
  f.setWeight(bodyFont.second);
  _lblB1->setFont(f);
  _lblB1->setText(tr("<html><br><br>• Enter the command ashirt will use to capture window screenshots<br><br>• Enter a key combination that will trigger the capture window command<html>"));

  auto hLayout = new QHBoxLayout();
  hLayout->addWidget(_lblB1, 0, Qt::AlignHCenter);

  auto _lblAKey = new QLabel(this);
  f.setPointSize(smallFont.first);
  f.setWeight(smallFont.second);
  _lblAKey->setFont(f);
  _lblAKey->setText(tr("Capture Window Command"));

  captureWindowLine = new QLineEdit(this);
  captureWindowLine->setTextMargins(3,0,3,0);
  registerField("command.window*", captureWindowLine);

  auto t1 = new QVBoxLayout();
  t1->setSpacing(1);
  t1->addWidget(_lblAKey);
  t1->addWidget(captureWindowLine);

  auto _lblBKey = new QLabel(this);
  f.setPointSize(smallFont.first);
  f.setWeight(smallFont.second);
  _lblBKey->setFont(f);
  _lblBKey->setText(tr("Shortcut"));

  captureWindowKeySequence = new SingleStrokeKeySequenceEdit(this);
  registerField("keySequence.window*", captureWindowKeySequence, "keySequence", SIGNAL(keySequenceChanged(const QKeySequence &)));

  auto t2 = new QVBoxLayout();
  t2->setSpacing(1);
  t2->addWidget(_lblBKey);
  t2->addWidget(captureWindowKeySequence);

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
