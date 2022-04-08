
include_directories(${Qt${QT_DEFAULT_MAJOR_VERSION}Gui_PRIVATE_INCLUDE_DIRS}/qpa)

if(APPLE)
    find_library(CARBON_LIBRARY Carbon)
endif()

add_library (HOTKEY_HELPERS STATIC
    hotkeymap.h
    uglobalhotkeys.cpp
    uglobalhotkeys.h
    ukeysequence.cpp
    ukeysequence.h
)
add_library(ASHIRT::HOTKEY_HELPERS ALIAS HOTKEY_HELPERS)

target_include_directories (HOTKEY_HELPERS PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

target_link_libraries ( HOTKEY_HELPERS PUBLIC
    Qt::Widgets
    Qt::Gui
    Qt::GuiPrivate
)

if(APPLE)
    target_link_libraries(HOTKEY_HELPERS PRIVATE ${CARBON_LIBRARY})
elseif(UNIX AND NOT APPLE)
    target_link_libraries(HOTKEY_HELPERS PRIVATE xcb xcb-keysyms)
elseif(WIN32)
    target_link_libraries(HOTKEY_HELPERS PRIVATE user32)
endif()
