
if(APPLE)
    find_library(CARBON_LIBRARY Carbon)
endif()

configure_file(releaseinfo.h.in releaseinfo.h @ONLY)

# QHotkey ships one backend per platform with no internal #ifdef guards, so only
# the source matching the current OS may be compiled.
if(APPLE)
    set(QHOTKEY_PLATFORM_SRC hotkeys/qhotkey_mac.cpp)
elseif(WIN32)
    set(QHOTKEY_PLATFORM_SRC hotkeys/qhotkey_win.cpp)
elseif(UNIX)
    set(QHOTKEY_PLATFORM_SRC hotkeys/qhotkey_x11.cpp)
endif()

add_library (HELPERS STATIC
    constants.h
    file_helpers.h
    http_status.h
    jsonhelpers.h
    multipartparser.cpp multipartparser.h
    netman.h
    request_builder.h
    screenshot.cpp screenshot.h
    cleanupreply.h
    string_helpers.h
    system_helpers.h
    ui_helpers.h
    hotkeys/qhotkey.cpp hotkeys/qhotkey.h hotkeys/qhotkey_p.h
    ${QHOTKEY_PLATFORM_SRC}
    ${CMAKE_CURRENT_BINARY_DIR}/releaseinfo.h
)

add_library(ASHIRT::HELPERS ALIAS HELPERS)

target_include_directories (HELPERS
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries ( HELPERS
    PUBLIC
      Qt::Network
      Qt::Widgets
      Qt::Gui
)

if(UNIX)
    find_package(Qt${QT_DEFAULT_MAJOR_VERSION} REQUIRED COMPONENTS DBus)
    target_link_libraries(HELPERS PRIVATE Qt::DBus)
endif()

if(APPLE)
    target_link_libraries(HELPERS PRIVATE ${CARBON_LIBRARY})
elseif(UNIX AND NOT APPLE)
    # QHotkey's X11 backend grabs keys via Xlib (XGrabKey) and filters xcb events.
    target_link_libraries(HELPERS PRIVATE X11 xcb)
elseif(WIN32)
    target_link_libraries(HELPERS PRIVATE user32)
endif()
