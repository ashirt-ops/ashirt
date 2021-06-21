# Generated from ashirt.pro.

cmake_minimum_required(VERSION 3.16.0)
project(ashirt VERSION 1.2.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(ENV{COMMIT_HASH} "")
set(ENV{VERSION_TAG} "")
set(ENV{SOURCE_CONTROL_REPO} "")

find_package(Qt6 COMPONENTS Widgets REQUIRED)
find_package(Qt6 COMPONENTS Gui REQUIRED)
find_package(Qt6 COMPONENTS Network REQUIRED)
find_package(Qt6 COMPONENTS Sql REQUIRED)

include_directories( "${CMAKE_CURRENT_BINARY_DIR}" )
include_directories(${PROJECT_SOURCE_DIR}/tools/UGlobalHotkey)

#####################################################################
## ashirt Binary:
#####################################################################
qt_add_executable(ashirt
        src/appconfig.h
        src/appsettings.h
        src/components/aspectratio_pixmap_label/aspectratiopixmaplabel.cpp src/components/aspectratio_pixmap_label/aspectratiopixmaplabel.h
        src/components/aspectratio_pixmap_label/imageview.cpp src/components/aspectratio_pixmap_label/imageview.h
        src/components/code_editor/codeblockview.cpp src/components/code_editor/codeblockview.h
        src/components/code_editor/codeeditor.cpp src/components/code_editor/codeeditor.h
        src/components/custom_keyseq_edit/singlestrokekeysequenceedit.cpp src/components/custom_keyseq_edit/singlestrokekeysequenceedit.h
        src/components/error_view/errorview.cpp src/components/error_view/errorview.h
        src/components/evidence_editor/deleteevidenceresponse.h
        src/components/evidence_editor/evidenceeditor.cpp src/components/evidence_editor/evidenceeditor.h
        src/components/evidence_editor/saveevidenceresponse.h
        src/components/evidencepreview.cpp src/components/evidencepreview.h
        src/components/flow_layout/flowlayout.cpp src/components/flow_layout/flowlayout.h
        src/components/loading/qprogressindicator.cpp src/components/loading/qprogressindicator.h
        src/components/loading_button/loadingbutton.cpp src/components/loading_button/loadingbutton.h
        src/components/tagging/tag_cache/tagcache.cpp src/components/tagging/tag_cache/tagcache.h
        src/components/tagging/tag_cache/tagcacheitem.cpp src/components/tagging/tag_cache/tagcacheitem.h
        src/components/tagging/tageditor.cpp src/components/tagging/tageditor.h
        src/components/tagging/tagginglineediteventfilter.h
        src/components/tagging/tagview.cpp src/components/tagging/tagview.h
        src/components/tagging/tagwidget.cpp src/components/tagging/tagwidget.h
        src/db/databaseconnection.cpp src/db/databaseconnection.h
        src/db/query_result.h
        src/dtos/checkConnection.h
        src/dtos/github_release.h
        src/dtos/operation.h
        src/dtos/tag.h
        src/exceptions/databaseerr.h
        src/exceptions/fileerror.h
        src/forms/add_operation/createoperation.cpp src/forms/add_operation/createoperation.h
        src/forms/credits/credits.cpp src/forms/credits/credits.h
        src/forms/evidence/evidencemanager.cpp src/forms/evidence/evidencemanager.h
        src/forms/evidence_filter/evidencefilter.cpp src/forms/evidence_filter/evidencefilter.h
        src/forms/evidence_filter/evidencefilterform.cpp src/forms/evidence_filter/evidencefilterform.h
        src/forms/getinfo/getinfo.cpp src/forms/getinfo/getinfo.h
        src/forms/porting/porting_dialog.cpp src/forms/porting/porting_dialog.h
        src/forms/settings/settings.cpp src/forms/settings/settings.h
        src/helpers/clipboard/clipboardhelper.cpp src/helpers/clipboard/clipboardhelper.h
        src/helpers/constants.h
        src/helpers/file_helpers.h
        src/helpers/http_status.h
        src/helpers/jsonhelpers.h
        src/helpers/multipartparser.cpp src/helpers/multipartparser.h
        src/helpers/netman.h
        src/helpers/request_builder.h
        src/helpers/screenshot.cpp src/helpers/screenshot.h
        src/helpers/stopreply.cpp src/helpers/stopreply.h
        src/helpers/system_helpers.h
        src/helpers/u_helpers.h
        src/hotkeymanager.cpp src/hotkeymanager.h
        src/main.cpp
        src/models/codeblock.cpp src/models/codeblock.h
        src/models/evidence.h
        src/models/tag.h
        src/porting/evidence_manifest.h
        src/porting/system_manifest.cpp src/porting/system_manifest.h
        src/porting/system_porting_options.h
        src/traymanager.cpp src/traymanager.h
        tools/UGlobalHotkey/hotkeymap.h
        tools/UGlobalHotkey/uglobal.h
        tools/UGlobalHotkey/uglobalhotkeys.cpp tools/UGlobalHotkey/uglobalhotkeys.h
        tools/UGlobalHotkey/ukeysequence.cpp tools/UGlobalHotkey/ukeysequence.h
)

target_include_directories(ashirt PUBLIC src tools/UGlobalHotkey)

target_link_libraries(ashirt PRIVATE Qt6::Widgets Qt6::Sql Qt6::Gui Qt6::Network)

# Resources:
set(res_migrations_resource_files
    "migrations/20200521190124-initial.sql"
    "migrations/20200521210407-add-screenshots-table.sql"
    "migrations/20200521210435-add-tags-table.sql"
    "migrations/20200625191727-support-codeblocks-p1.sql"
    "migrations/20200625192018-support-codeblocks-p2.sql"
    "migrations/20200625192444-support-codeblocks-p3.sql"
    "migrations/20200625203249-support-codeblocks-p4.sql"
)

qt_add_resources(ashirt "res_migrations"
    PREFIX
        "/"
    FILES
        ${res_migrations_resource_files}
)
set(res_icons_resource_files
    "icons/shirt-dark.svg"
    "icons/shirt-light.svg"
    "icons/shirt-red.svg"
    "icons/windowIcon.png"
)

qt_add_resources(ashirt "res_icons"
    PREFIX
        "/"
    FILES
        ${res_icons_resource_files}
)


#### Keys ignored in scope 1:.:.:ashirt.pro:<TRUE>:
# TEMPLATE = "app"

## Scopes:
#####################################################################

#### Keys ignored in scope 2:.:.:ashirt.pro:NOT VERSION_TAG_PLAIN___contains___._x_tags/v._x_:
# VERSION_TAG_PLAIN = "v0.0.0-development"

#### Keys ignored in scope 3:.:.:ashirt.pro:COMMIT_HASH_PLAIN___equals___:
# COMMIT_HASH_PLAIN = "Unknown"

#### Keys ignored in scope 5:.:.:ashirt.pro:MACOS:
# ICON = "icons/ashirt.icns"
# QMAKE_TARGET_BUNDLE_PREFIX = "com.theparanoids"

#### Keys ignored in scope 6:.:.:ashirt.pro:UNIX:
# INSTALLS = "target" "desktop" "icons"
# desktop.files = "linux/ashirt.desktop"
# desktop.path = "$$DATADIR/applications"
# icons.files = "linux/icons/*"
# icons.path = "$$DATADIR/icons/hicolor"
# target.path = "$$BINDIR"

#### Keys ignored in scope 7:.:.:ashirt.pro:PREFIX_ISEMPTY:
# PREFIX = "/usr"

#### Keys ignored in scope 8:.:.:ashirt.pro:BINDIR_ISEMPTY:
# BINDIR = "$$PREFIX/bin"

#### Keys ignored in scope 9:.:.:ashirt.pro:DATADIR_ISEMPTY:
# DATADIR = "$$PREFIX/share"

#### Keys ignored in scope 10:.:.:ashirt.pro:QNX:
# target.path = "/tmp/$${TARGET}/bin"

#### Keys ignored in scope 11:.:.:ashirt.pro:NOT target.path_ISEMPTY:
# INSTALLS = "target"

#qt_extend_target(ashirt CONDITION LINUX
#    PUBLIC_LIBRARIES
#        Qt::GuiPrivate
#        XCB::XCB
#        xcb-keysyms
#)#

#qt_extend_target(ashirt CONDITION APPLE
#    PUBLIC_LIBRARIES
#        ${FWCarbon}
#)#

#qt_extend_target(ashirt CONDITION GCC AND windows
#    PUBLIC_LIBRARIES
#        user32
#)#

#qt_extend_target(ashirt CONDITION _x_-msvc_x_ AND windows
#    PUBLIC_LIBRARIES
#        user32.lib
#)
