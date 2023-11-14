#include "firstTimeWizard.h"

#include <QDesktopServices>
#include <QGridLayout>
#include <QKeySequenceEdit>
#include <QLabel>
#include <QLine>
#include <QPainter>
#include <QPushButton>
#include <releaseinfo.h>
#include <appconfig.h>
#include <QFileDialog>

#include "requirementspage.h"
#include "evidencepage.h"
#include "apikeyspage.h"
#include "hostpathpage.h"
#include "hosttestpage.h"
#include "captureareapage.h"

#include "capturewindowpage.h"
#include "captureclipboardpage.h"
#include "opspage.h"
#include "finishedpage.h"

FirstTimeWizard::FirstTimeWizard()
{
    setFixedSize(780, 480);
    setWizardStyle(ModernStyle);

    setOption(QWizard::NoCancelButton, true);
    setOption(QWizard::NoDefaultButton, true);
    setPage(WizardPage::Page_Requiments, new RequirementsPage(this));
    setPage(WizardPage::Page_Evidence, new EvidencePage(this));
    setPage(WizardPage::Page_HostPath, new HostPathPage(this));
    setPage(WizardPage::Page_Api, new ApiKeysPage(this));
    setPage(WizardPage::Page_HostTest, new HostTestPage(this));
    setPage(WizardPage::Page_CaptureArea, new CaptureAreaPage(this));
    setPage(WizardPage::Page_CaptureWindow, new CaptureWindowPage(this));
    setPage(WizardPage::Page_CaptureClipboard, new CaptureClipboardPage(this));
    setPage(WizardPage::Page_Ops, new OpsPage(this));
    setPage(WizardPage::Page_Finished, new FinishedPage(this));
    setStartId(WizardPage::Page_Requiments);
}
