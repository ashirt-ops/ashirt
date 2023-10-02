set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_subdirectory(components)
add_subdirectory(db)
add_subdirectory(dtos)
add_subdirectory(forms)
add_subdirectory(helpers)
add_subdirectory(models)
add_subdirectory(porting)

set(ASHIRT_SOURCES
     appconfig.cpp appconfig.h
     hotkeymanager.cpp hotkeymanager.h
     main.cpp
     traymanager.cpp traymanager.h
     ${CMAKE_SOURCE_DIR}/icons/res_icons.qrc
 )
## Locate the deploy app for later use
get_target_property(qmake_executable Qt${QT_DEFAULT_MAJOR_VERSION}::qmake IMPORTED_LOCATION)
get_filename_component(_qt_bin_dir "${qmake_executable}" DIRECTORY)
if(WIN32 OR APPLE)
    if(WIN32)
        set(deployqtapp windeployqt)
    elseif(APPLE)
        set(deployqtapp macdeployqt)
    endif()
    find_program(PLATFORMDEPLOYQT ${deployqtapp} HINTS "${_qt_bin_dir}")
endif()

if(APPLE)
    set(ASHIRT_PLATFORM_EX_SRC ${CMAKE_SOURCE_DIR}/deploy/ashirt.icns)
    set_source_files_properties(${ASHIRT_PLATFORM_EX_SRC} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
elseif(WIN32)
    set(CMAKE_PREFIX_PATH $ENV{QTDIR})
    include_directories(${CMAKE_SOURCE_DIR}/deploy)
    set(ASHIRT_PLATFORM_EX_SRC ${CMAKE_BINARY_DIR}/deploy/ashirt.rc)
endif()

#MACOSX_BUNDLE Make it a bundle on Mac OS
#WIN32 Its a "GUI" app on win32
add_executable(ashirt MACOSX_BUNDLE WIN32 ${ASHIRT_SOURCES} ${ASHIRT_PLATFORM_EX_SRC})
set_target_properties(ashirt PROPERTIES
    MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/deploy/ashirt.plist.in
    MACOSX_BUNDLE_GUI_IDENTIFIER "com.theparanoids.ashirt"
    MACOSX_BUNDLE_BUNDLE_NAME "ashirt"
    MACOSX_BUNDLE_ICON_FILE ashirt.icns
    MACOSX_BUNDLE_BUNDLE_VERSION ${CMAKE_PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING "${CMAKE_PROJECT_VERSION}"
)

target_link_libraries ( ashirt
    PRIVATE
        ASHIRT::COMPONENTS
        ASHIRT::HELPERS
        ASHIRT::DTOS
        ASHIRT::FORMS
        ASHIRT::PORTING
)

if(APPLE)
    if(NOTARIZE_AS STREQUAL "")
        add_custom_command(
            TARGET ashirt POST_BUILD
            COMMAND ${PLATFORMDEPLOYQT} ${CMAKE_CURRENT_BINARY_DIR}/ashirt.app
        )
    else()
        message(STATUS "Sign Bundle As: ${NOTARIZE_AS}")
        set(HOMEBREW_CELLAR "/usr/local/Cellar")
        if(EXISTS /opt/homebrew/Cellar)
            set(HOMEBREW_CELLAR "/opt/homebrew/Cellar")
        endif()
        add_custom_command(
            TARGET ashirt POST_BUILD
            COMMAND mkdir -p ${CMAKE_CURRENT_BINARY_DIR}/ashirt.app/Contents/Frameworks
            COMMAND install ${HOMEBREW_CELLAR}/brotli/1.1.0/lib/libbrotlicommon.1.1.0.dylib ${CMAKE_CURRENT_BINARY_DIR}/ashirt.app/Contents/Frameworks/libbrotlicommon.1.dylib
            COMMAND codesign -v --timestamp --sign ${NOTARIZE_AS} ${CMAKE_CURRENT_BINARY_DIR}/ashirt.app/Contents/Frameworks/libbrotlicommon.1.dylib
            COMMAND ${PLATFORMDEPLOYQT} ${CMAKE_CURRENT_BINARY_DIR}/ashirt.app -sign-for-notarization=${NOTARIZE_AS}
        )
    endif()
    install(TARGETS ashirt BUNDLE DESTINATION .)
elseif(UNIX AND NOT APPLE)
    install(TARGETS ashirt BUNDLE DESTINATION bin)
elseif(WIN32)
    install(TARGETS ashirt RUNTIME DESTINATION .)
    add_custom_command(
        TARGET ashirt POST_BUILD
        COMMAND ${PLATFORMDEPLOYQT} $<TARGET_FILE:ashirt> --no-compiler-runtime --no-system-d3d-compiler --no-quick-import --no-translations --no-opengl-sw --dir ${CMAKE_BINARY_DIR}/qtDeploy --plugindir ${CMAKE_BINARY_DIR}/qtDeploy/plugins
    )
endif()
