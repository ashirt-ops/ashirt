#include "evidencepage.h"

#include <QFileDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <appconfig.h>

bool EvidencePage::validatePage()
{
  QDir path = field("eRepo").toString();

  if (!path.exists())
    path.mkpath(path.absolutePath());

  if(!path.exists()) {
    errorLabel->setText(tr("can not use selected path"));
    return false;
  }
  AppConfig::setValue(CONFIG::EVIDENCEREPO, path.absolutePath());
  errorLabel->setText(QString());
  return true;
}

void EvidencePage::initializePage()
{
  QString repoPath = AppConfig::value(CONFIG::EVIDENCEREPO);
  setField("eRepo", repoPath);
  evidenceLine->setText(repoPath);

  if(repoPath.isEmpty())
    errorLabel->setText(tr("Enter a Path"));
  QDir repo(repoPath);

  if (!repo.exists()) {
    repo.mkpath(repo.absolutePath());
  }

  if(!repo.exists() || !repo.isReadable())
    errorLabel->setText(tr("Invalid Path"));
}

EvidencePage::EvidencePage(QWidget *parent)
    : WizardPage{Page_Evidence, parent}
{
  auto f = font();

  auto _lblTitleLabel = new QLabel(this);
  f.setPointSize(titleFont.first);
  f.setWeight(titleFont.second);
  _lblTitleLabel->setFont(f);
  _lblTitleLabel->setText(tr("Evidence Repository"));

  auto _lblSubtitle = new QLabel(this);
  f.setPointSize(subTitleFont.first);
  f.setWeight(subTitleFont.second);
  _lblSubtitle->setFont(f);
  _lblSubtitle->setWordWrap(true);
  _lblSubtitle->setText(tr("This is where evidence is stored and your jumping off point.\n Files will be stored in a subdirectory using the operation name."));

  auto _lblEvidence = new QLabel(this);
  f.setPointSize(smallFont.first);
  f.setWeight(smallFont.second);
  _lblEvidence->setFont(f);
  _lblEvidence->setText(tr("Evidence Repository:"));

  evidenceLine = new QLineEdit(this);
  evidenceLine->setTextMargins(3,0,3,0);
  registerField("eRepo*", evidenceLine);

  auto _btnRepoBrowse = new QPushButton(this);
  _btnRepoBrowse->setText(tr("Browse"));
  connect(_btnRepoBrowse, &QPushButton::clicked, this, [this] {
    QString txt = QFileDialog::getExistingDirectory(this, tr("Select Path to Store Evidence"), evidenceLine->text());
    if(txt.isEmpty())
      return;
    evidenceLine->setText(txt);
    setField("eRepo", txt);
  });

  auto _btnReset = new QPushButton(this);
  _btnReset->setText(tr("Restore Default"));
  connect(_btnReset, &QPushButton::clicked, this, [this] {
    evidenceLine->setText(AppConfig::defaultValue(CONFIG::EVIDENCEREPO));
    setField("eRepo", evidenceLine->text());
  });

  errorLabel = new QLabel(this);
  f.setPointSize(smallFont.first);
  f.setWeight(smallFont.second);
  errorLabel->setFont(f);

  auto btnLayout = new QHBoxLayout();
  btnLayout->addWidget(evidenceLine);
  btnLayout->addWidget(_btnRepoBrowse);
  btnLayout->addWidget(_btnReset);

  auto t1 = new QVBoxLayout();
  t1->setSpacing(1);
  t1->addWidget(_lblEvidence);
  t1->addLayout(btnLayout);
  t1->addWidget(errorLabel);

  auto _lblBody = new QLabel(this);
  f.setPointSize(bodyFont.first);
  f.setWeight(bodyFont.second);
  _lblBody->setFont(f);
  _lblBody->setText(tr("• Browse and input operation filename"));

  auto layout = new QVBoxLayout();
  layout->addWidget(_lblTitleLabel);
  layout->addWidget(_lblSubtitle);
  layout->addSpacerItem(new QSpacerItem(0,30,QSizePolicy::Minimum, QSizePolicy::Fixed));
  layout->addWidget(_lblBody);
  layout->addSpacerItem(new QSpacerItem(0,40,QSizePolicy::Minimum, QSizePolicy::Fixed));
  layout->addLayout(t1);
  layout->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Minimum, QSizePolicy::Expanding));
  setLayout(layout);

}
