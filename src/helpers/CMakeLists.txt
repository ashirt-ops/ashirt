
include_directories(${Qt${QT_DEFAULT_MAJOR_VERSION}Gui_PRIVATE_INCLUDE_DIRS}/qpa)

if(APPLE)
    find_library(CARBON_LIBRARY Carbon)
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
    stopreply.cpp stopreply.h
    system_helpers.h
    ui_helpers.h
    clipboard/clipboardhelper.cpp clipboard/clipboardhelper.h
    hotkeys/hotkeymap.h
    hotkeys/uglobalhotkeys.cpp hotkeys/uglobalhotkeys.h
    hotkeys/ukeysequence.cpp hotkeys/ukeysequence.h
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
      Qt::GuiPrivate
)

if(APPLE)
    target_link_libraries(HELPERS PRIVATE ${CARBON_LIBRARY})
elseif(UNIX AND NOT APPLE)
    target_link_libraries(HELPERS PRIVATE xcb xcb-keysyms pthread)
elseif(WIN32)
    target_link_libraries(HELPERS PRIVATE user32)
endif()
