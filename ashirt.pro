QT       += core gui network sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# App version number
VERSION_TAG_PLAIN = $$getenv(GITHUB_REF)
COMMIT_HASH_PLAIN = $$getenv(GITHUB_SHA)

!contains(VERSION_TAG_PLAIN, .*tags/v.*) {
  message("Ref appears to not be a tag (Value: $$VERSION_TAG_PLAIN). Using non-version instead.")
  VERSION_TAG_PLAIN = 0.0.0-Unversioned
}

equals(COMMIT_HASH_PLAIN, "") {
  message("commit hash specified. Please ensure GITHUB_SHA environment variable is set.")
  COMMIT_HASH_PLAIN = Unknown
}


VERSION_TAG = \\\"$$VERSION_TAG_PLAIN\\\"
COMMIT_HASH = \\\"$$COMMIT_HASH_PLAIN\\\"

message(Building version [$$VERSION_TAG_PLAIN])

DEFINES += "VERSION_TAG=$$VERSION_TAG" \
           "COMMIT_HASH=$$COMMIT_HASH"

INCLUDEPATH += src

SOURCES += \
    src/components/aspectratio_pixmap_label/aspectratiopixmaplabel.cpp \
    src/components/aspectratio_pixmap_label/imageview.cpp \
    src/components/code_editor/codeblockview.cpp \
    src/components/code_editor/codeeditor.cpp \
    src/components/custom_keyseq_edit/singlestrokekeysequenceedit.cpp \
    src/components/error_view/errorview.cpp \
    src/components/evidence_editor/evidenceeditor.cpp \
    src/components/evidencepreview.cpp \
    src/components/flow_layout/flowlayout.cpp \
    src/components/loading/qprogressindicator.cpp \
    src/components/loading_button/loadingbutton.cpp \
    src/components/tagging/tageditor.cpp \
    src/components/tagging/tagview.cpp \
    src/components/tagging/tagwidget.cpp \
    src/db/databaseconnection.cpp \
    src/forms/evidence_filter/evidencefilter.cpp \
    src/forms/evidence_filter/evidencefilterform.cpp \
    src/forms/getinfo/getinfo.cpp \
    src/helpers/clipboard/clipboardhelper.cpp \
    src/models/codeblock.cpp \
    src/helpers/multipartparser.cpp \
    src/hotkeymanager.cpp \
    src/main.cpp \
    src/traymanager.cpp \
    src/helpers/screenshot.cpp \
    src/helpers/stopreply.cpp \
    src/forms/credits/credits.cpp \
    src/forms/evidence/evidencemanager.cpp \
    src/forms/settings/settings.cpp

HEADERS += \
    src/components/aspectratio_pixmap_label/aspectratiopixmaplabel.h \
    src/components/aspectratio_pixmap_label/imageview.h \
    src/components/code_editor/codeblockview.h \
    src/components/code_editor/codeeditor.h \
    src/components/custom_keyseq_edit/singlestrokekeysequenceedit.h \
    src/components/error_view/errorview.h \
    src/components/evidence_editor/deleteevidenceresponse.h \
    src/components/evidence_editor/evidenceeditor.h \
    src/components/evidence_editor/saveevidenceresponse.h \
    src/components/evidencepreview.h \
    src/components/flow_layout/flowlayout.h \
    src/components/loading/qprogressindicator.h \
    src/components/loading_button/loadingbutton.h \
    src/components/tagging/tageditor.h \
    src/components/tagging/tagginglineediteventfilter.h \
    src/components/tagging/tagview.h \
    src/components/tagging/tagwidget.h \
    src/db/databaseconnection.h \
    src/exceptions/databaseerr.h \
    src/exceptions/fileerror.h \
    src/forms/evidence_filter/evidencefilter.h \
    src/forms/evidence_filter/evidencefilterform.h \
    src/forms/getinfo/getinfo.h \
    src/helpers/clipboard/clipboardhelper.h \
    src/helpers/ui_helpers.h \
    src/models/codeblock.h \
    src/helpers/file_helpers.h \
    src/helpers/http_status.h \
    src/hotkeymanager.h \
    src/models/evidence.h \
    src/models/tag.h \
    src/traymanager.h \
    src/appconfig.h \
    src/appsettings.h \
    src/helpers/jsonhelpers.h \
    src/helpers/multipartparser.h \
    src/helpers/netman.h \
    src/helpers/pathseparator.h \
    src/helpers/screenshot.h \
    src/helpers/stopreply.h \
    src/dtos/tag.h \
    src/dtos/operation.h \
    src/forms/credits/credits.h \
    src/forms/evidence/evidencemanager.h \
    src/forms/settings/settings.h

FORMS += \
    src/forms/credits/credits.ui \
    src/forms/evidence_filter/evidencefilterform.ui \
    src/forms/getinfo/getinfo.ui

include(tools/UGlobalHotkey/uglobalhotkey.pri)

macx {
  ICON = icons/ashirt.icns
}

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    bin/update_migration_resource.py

RESOURCES += \
    res_migrations.qrc \
    res_icons.qrc
