

if(WIN32)
    configure_file(ashirt.rc.in ${CMAKE_CURRENT_BINARY_DIR}/ashirt.rc @ONLY)
    install(FILES ${CMAKE_SOURCE_DIR}/LICENSE DESTINATION . RENAME LICENSE.txt)
    install(FILES ${CMAKE_SOURCE_DIR}/README.md DESTINATION . RENAME README.txt)
elseif(APPLE)
    install(FILES ${CMAKE_SOURCE_DIR}/README.md DESTINATION .)
elseif(UNIX AND NOT APPLE)
    install(DIRECTORY hicolor DESTINATION share/icons)
    install(FILES ashirt.desktop DESTINATION share/applictions)
endif()

##CPACK
set(CPACK_STRIP_FILES TRUE)
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
set(CPACK_PACKAGE_NAME "ashirt")
if(NOT CPACK_PACKAGE_VERSION)
    set(CPACK_PACKAGE_VERSION ${CMAKE_PROJECT_VERSION})
endif()
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}")
if(CMAKE_SYSTEM_NAME MATCHES "Windows")
    set(CPACK_PACKAGE_VENDOR "Yahoo")
    set(CPACK_PACKAGE_INSTALL_DIRECTORY "ashirt")
    set(CPACK_NSIS_DISPLAY_NAME ${CMAKE_PACKAGE_NAME})
    set(CPACK_NSIS_COMPRESSOR lzma)
    set(CPACK_NSIS_MUI_ICON "${CMAKE_CURRENT_SOURCE_DIR}/ashirt.ico")
    set(CPACK_NSIS_INSTALLED_ICON_NAME ashirt.exe)
    set(CPACK_NSIS_MENU_LINKS "ashirt.exe" "ashirt")
    set(CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS
        "RMDir /r '$INSTDIR'"
    )
    install(
        DIRECTORY ${CMAKE_BINARY_DIR}/qtDeploy/
        DESTINATION .
        FILES_MATCHING PATTERN "*.*"
    )
    set(CPACK_GENERATOR "NSIS")
elseif(CMAKE_SYSTEM_NAME MATCHES "Darwin")
    # Write our cert identifier to a file generate_ds_store.applescript can read
    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/NOTARIZE_AS ${NOTARIZE_AS})
    configure_file(generate_ds_store.applescript.in ${CMAKE_CURRENT_BINARY_DIR}/generate_ds_store.applescript @ONLY)
    set(CPACK_PACKAGE_ICON "${CMAKE_CURRENT_SOURCE_DIR}/dmg_icon.icns")
    set(CPACK_DMG_BACKGROUND_IMAGE "${CMAKE_CURRENT_SOURCE_DIR}/dmg_background.png")
    set(CPACK_DMG_DS_STORE_SETUP_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/generate_ds_store.applescript")
    set(CPACK_DMG_VOLUME_NAME "ashirt")
    set(CPACK_GENERATOR "DragNDrop")
endif()

INCLUDE (CPack)
