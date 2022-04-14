

if(WIN32)
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
    if(QT_DEFAULT_MAJOR_VERSION MATCHES "5")
        STRING(APPEND CPACK_PACKAGE_FILE_NAME "-Qt5")
    endif()
if(CMAKE_SYSTEM_NAME MATCHES "Windows")
    set(CPACK_PACKAGE_VENDOR "Yahoo")
    set(CPACK_PACKAGE_INSTALL_DIRECTORY "ashirt")
    set(CPACK_NSIS_DISPLAY_NAME ${CMAKE_PACKAGE_NAME})
    set(CPACK_NSIS_COMPRESSOR lzma)
    set(CPACK_NSIS_MUI_ICON "${PROJECT_SOURCE_DIR}/deploy/ashirt.ico")
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
    file(WRITE ${PROJECT_SOURCE_DIR}/deploy/NOTARIZE_AS ${NOTARIZE_AS})
    set(CPACK_PACKAGE_ICON "${PROJECT_SOURCE_DIR}/deploy/dmg_icon.icns")
    set(CPACK_DMG_BACKGROUND_IMAGE "${PROJECT_SOURCE_DIR}/deploy/dmg_background.png")
    set(CPACK_DMG_DS_STORE_SETUP_SCRIPT "${PROJECT_SOURCE_DIR}/deploy/generate_ds_store.applescript")
    set(CPACK_DMG_VOLUME_NAME "ashirt")
    set(CPACK_GENERATOR "DragNDrop")
endif()

INCLUDE (CPack)
