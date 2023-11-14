
add_library (FORMS STATIC
    add_operation/createoperation.cpp add_operation/createoperation.h
    ashirtdialog/ashirtdialog.cpp ashirtdialog/ashirtdialog.h
    credits/credits.cpp credits/credits.h
    evidence/evidencemanager.cpp evidence/evidencemanager.h
    evidence_filter/evidencefilter.cpp evidence_filter/evidencefilter.h
    evidence_filter/evidencefilterform.cpp evidence_filter/evidencefilterform.h
    getinfo/getinfo.cpp getinfo/getinfo.h
    porting/porting_dialog.cpp porting/porting_dialog.h
    settings/settings.cpp settings/settings.h
    firstRunWizard/firstTimeWizard.h firstRunWizard/firstTimeWizard.cpp
    firstRunWizard/welcomepage.h firstRunWizard/welcomepage.cpp
    firstRunWizard/wizardpage.h firstRunWizard/wizardpage.cpp
    firstRunWizard/requirementspage.h firstRunWizard/requirementspage.cpp
    firstRunWizard/evidencepage.h firstRunWizard/evidencepage.cpp
    firstRunWizard/apikeyspage.h firstRunWizard/apikeyspage.cpp
    firstRunWizard/hostpathpage.h firstRunWizard/hostpathpage.cpp
    firstRunWizard/hosttestpage.h firstRunWizard/hosttestpage.cpp
    firstRunWizard/captureareapage.h firstRunWizard/captureareapage.cpp
    firstRunWizard/capturewindowpage.h firstRunWizard/capturewindowpage.cpp
    firstRunWizard/captureclipboardpage.h firstRunWizard/captureclipboardpage.cpp
    firstRunWizard/opspage.h firstRunWizard/opspage.cpp
    firstRunWizard/finishedpage.h firstRunWizard/finishedpage.cpp
)

add_library(ASHIRT::FORMS ALIAS FORMS)

target_include_directories (FORMS
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries ( FORMS
    PUBLIC
      Qt::Gui
      Qt::Widgets
      Qt::Sql
      Qt::Svg
      ASHIRT::HELPERS
      ASHIRT::MODELS
      ASHIRT::COMPONENTS
)
