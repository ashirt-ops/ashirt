
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
      ASHIRT::HELPERS
      ASHIRT::MODELS
      ASHIRT::COMPONENTS
)
