cmake_minimum_required(VERSION 3.18)

project(ashirt VERSION 1.2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

if(EXISTS ${CMAKE_SOURCE_DIR}/.git)
    find_package(Git)
    if(GIT_FOUND)
     EXECUTE_PROCESS(
      COMMAND ${GIT_EXECUTABLE} describe --long --match v*
      WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
      OUTPUT_VARIABLE GITREV
      ERROR_QUIET
      OUTPUT_STRIP_TRAILING_WHITESPACE)
     string(REGEX MATCH [0-9]+ MAJOR ${GITREV})
     string(REGEX MATCH \\.[0-9]+ MINOR ${GITREV})
     string(REPLACE "." "" MINOR ${MINOR})
     string(REGEX MATCH [0-9]+\- PATCH ${GITREV})
     string(REPLACE "-" "" PATCH ${PATCH})
     string(REGEX MATCH \-[0-9]+\- TWEAK ${GITREV})
     string(REPLACE "-" "" TWEAK ${TWEAK})
     set(CMAKE_PROJECT_VERSION_MAJOR ${MAJOR})
     set(CMAKE_PROJECT_VERSION_MINOR ${MINOR})
     set(CMAKE_PROJECT_VERSION_PATCH ${PATCH})
     set(CMAKE_PROJECT_VERSION_TWEAK ${TWEAK})
     set(CMAKE_PROJECT_VERSION "${MAJOR}.${MINOR}.${PATCH}.${TWEAK}")
     add_definitions(-DVERSION_TAG="v${MAJOR}.${MINOR}.${PATCH}")
     EXECUTE_PROCESS(
      COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
      WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
      OUTPUT_VARIABLE GITHASH
      ERROR_QUIET
      OUTPUT_STRIP_TRAILING_WHITESPACE)
     add_definitions(-DCOMMIT_HASH="${GITHASH}")
    EXECUTE_PROCESS(
      COMMAND ${GIT_EXECUTABLE} remote get-url origin
      WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
      OUTPUT_VARIABLE GITORIGIN
      ERROR_QUIET
      OUTPUT_STRIP_TRAILING_WHITESPACE)
    add_definitions(-DSOURCE_CONTROL_REPO="${GITORIGIN}")
    endif()
else()
    add_definitions(-DCOMMIT_HASH="")
    add_definitions(-DVERSION_TAG="")
    add_definitions(-DSOURCE_CONTROL_REPO="")
    add_definitions(-DVERSION_TAG_PLAIN "v0.0.0-development")
    add_definitions(-DCOMMIT_HASH_PLAIN "Unknown")
endif()
     message(STATUS "VERSION: ${CMAKE_PROJECT_VERSION}")

if(NOT QT_DEFAULT_MAJOR_VERSION)
    set(QT_DEFAULT_MAJOR_VERSION 5 CACHE STRING "" FORCE)
endif()

find_package(Qt${QT_DEFAULT_MAJOR_VERSION} REQUIRED COMPONENTS
    Widgets
    Gui
    Network
    Sql
    Core
)
include_directories(${CMAKE_SOURCE_DIR}/src)

add_subdirectory(src/helpers/hotkeys)
set(ASHIRT_SOURCES
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
    src/helpers/ui_helpers.h
    src/hotkeymanager.cpp src/hotkeymanager.h
    src/main.cpp
    src/models/codeblock.cpp src/models/codeblock.h
    src/models/evidence.h
    src/models/tag.h
    src/porting/evidence_manifest.h
    src/porting/system_manifest.cpp src/porting/system_manifest.h
    src/porting/system_porting_options.h
    src/traymanager.cpp src/traymanager.h
    res_icons.qrc
    res_migrations.qrc
)

if(APPLE)
    set(ASHIRT_PLATFORM_EX_SRC ${CMAKE_SOURCE_DIR}/icons/ashirt.icns)
    set_source_files_properties(${ASHIRT_PLATFORM_EX_SRC} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
elseif(WIN32)
    set(CMAKE_PREFIX_PATH $ENV{QTDIR})
    set(ASHIRT_PLATFORM_EX_SRC ${CMAKE_SOURCE_DIR}/windows/ashirt.rc)
endif()

#MACOSX_BUNDLE Make it a bundle on Mac OS
#WIN32 Its a "GUI" app on win32
add_executable(ashirt MACOSX_BUNDLE WIN32 ${ASHIRT_SOURCES} ${ASHIRT_PLATFORM_EX_SRC})
set_target_properties(ashirt PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER "com.theparanoids.ashirt"
    MACOSX_BUNDLE_BUNDLE_NAME "ashirt"
    MACOSX_BUNDLE_ICON_FILE ashirt.icns
    MACOSX_BUNDLE_BUNDLE_VERSION ${CMAKE_PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING "${CMAKE_PROJECT_VERSION}"
    )

target_link_libraries ( ashirt
    PRIVATE
        Qt::Widgets
        Qt::Sql
        Qt::Gui
        Qt::Network
        ASHIRT::HOTKEY_HELPERS
)

if(UNIX AND NOT APPLE)
    target_link_libraries(ashirt PRIVATE pthread)
endif()

if(UNIX)
install(TARGETS ashirt
          BUNDLE DESTINATION .
          RUNTIME DESTINATION bin
)
elseif(WIN32)
    install(TARGETS ashirt RUNTIME DESTINATION .)
endif()

if(UNIX AND NOT APPLE)
    install(FILES ${CMAKE_SOURCE_DIR}/linux/icons/* DESTINATION share/icons/hicolor)
    install(FILES ${CMAKE_SOURCE_DIR}/linux/ashirt.desktop DESTINATION share/applictions)
endif()

##CPACK
set(CPACK_STRIP_FILES TRUE)
get_target_property(_qmake_executable Qt${QT_DEFAULT_MAJOR_VERSION}::qmake IMPORTED_LOCATION)
get_filename_component(_qt_bin_dir "${_qmake_executable}" DIRECTORY)
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
set(CPACK_PACKAGE_NAME "ashirt")
set(CPACK_PACKAGE_VERSION ${CMAKE_PROJECT_VERSION})

if(CMAKE_SYSTEM_NAME MATCHES "Windows")
    set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-Qt${QT_DEFAULT_MAJOR_VERSION}-Windows")
    set(CPACK_PACKAGE_INSTALL_DIRECTORY "ashirt")
    set(CPACK_NSIS_DISPLAY_NAME ${CMAKE_PACKAGE_NAME})
    set(CPACK_NSIS_COMPRESSOR lzma)
    set(CPACK_NSIS_MUI_ICON "${PROJECT_SOURCE_DIR}/windows/ashirt.ico")
    set(CPACK_NSIS_INSTALLED_ICON_NAME ashirt.exe)
    set(CPACK_NSIS_MENU_LINKS "ashirt.exe" "ashirt")
    set(CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS
        "RMDir /r '$INSTDIR'"
    )
    set(CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION .)
    set(CMAKE_INSTALL_UCRT_LIBRARIES TRUE)
    include(InstallRequiredSystemLibraries)
    find_program(WINDEPLOYQT windeployqt HINTS "${_qt_bin_dir}")
    if(${QT_DEFAULT_MAJOR_VERSION} MATCHES "5")
      add_custom_command(
          TARGET ashirt POST_BUILD
          COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/qtDeploy
          COMMAND ${WINDEPLOYQT} --plugindir ${CMAKE_BINARY_DIR}/qtDeploy/plugins --no-compiler-runtime --no-system-d3d-compiler --no-angle --no-webkit2 --no-quick-import --no-translations --dir ${CMAKE_BINARY_DIR}/qtDeploy $<TARGET_FILE:ashirt>
      )
    else()
      add_custom_command(
          TARGET ashirt POST_BUILD
          COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/qtDeploy
          COMMAND ${WINDEPLOYQT} --plugindir ${CMAKE_BINARY_DIR}/qtDeploy/plugins --no-compiler-runtime --no-system-d3d-compiler --no-quick-import --no-translations --dir ${CMAKE_BINARY_DIR}/qtDeploy $<TARGET_FILE:ashirt>
      )
    endif()

    install(
        DIRECTORY ${CMAKE_BINARY_DIR}/qtDeploy/
        DESTINATION .
        FILES_MATCHING PATTERN "*.*"
    )
    set(CPACK_GENERATOR "ZIP;NSIS")
endif()

INCLUDE (CPack)
