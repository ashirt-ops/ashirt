

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

# Packaging metadata shared by every platform's packaging step (CPack on
# macOS/Linux, WiX on Windows). PACKAGE_VERSION may be passed on the command
# line (a release tag like "v1.2.3" or "continuous"); otherwise it defaults to
# the git-derived project version.
set(PACKAGE_NAME "ashirt")
if(NOT PACKAGE_VERSION)
    set(PACKAGE_VERSION ${CMAKE_PROJECT_VERSION})
endif()
set(PACKAGE_FILE_NAME "${PACKAGE_NAME}-${PACKAGE_VERSION}")
if(CMAKE_SYSTEM_NAME MATCHES "Windows")
    install(
        DIRECTORY ${CMAKE_BINARY_DIR}/qtDeploy/
        DESTINATION .
        FILES_MATCHING PATTERN "*.*"
    )

    # Windows packaging is handled by the modern WiX toolset (the `wix` CLI,
    # WixToolset.Sdk 7.x) rather than CPack/NSIS. `cmake --build . --target
    # package` stages the install tree and harvests it into a single MSI.

    # Derive a numeric MSI ProductVersion from the release tag (e.g. "v1.2.3" ->
    # "1.2.3.0"); fall back to the project version for untagged/continuous builds.
    if(PACKAGE_VERSION MATCHES "^v?([0-9]+\\.[0-9]+\\.[0-9]+)$")
        set(MSI_PRODUCT_VERSION "${CMAKE_MATCH_1}.0")
    else()
        set(MSI_PRODUCT_VERSION "${PROJECT_VERSION}.0")
    endif()
    message(STATUS "MSI ProductVersion: ${MSI_PRODUCT_VERSION}")

    configure_file(ashirt.wxs.in ${CMAKE_CURRENT_BINARY_DIR}/ashirt.wxs @ONLY)

    set(WIX_STAGE_DIR ${CMAKE_BINARY_DIR}/installRoot)
    set(WIX_OUTPUT ${CMAKE_BINARY_DIR}/${PACKAGE_FILE_NAME}.msi)

    add_custom_target(package
        COMMAND ${CMAKE_COMMAND} -E rm -rf ${WIX_STAGE_DIR}
        COMMAND ${CMAKE_COMMAND} --install ${CMAKE_BINARY_DIR} --prefix ${WIX_STAGE_DIR} --config $<CONFIG>
        COMMAND wix build -arch x64 -pdbtype none
                -acceptEula wix7
                -ext WixToolset.UI.wixext
                -b InstallRoot=${WIX_STAGE_DIR}
                ${CMAKE_CURRENT_BINARY_DIR}/ashirt.wxs
                -o ${WIX_OUTPUT}
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Building MSI installer with WiX: ${WIX_OUTPUT}"
        VERBATIM
    )
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

# Windows uses the WiX `package` target above; every other platform packages
# through CPack, which consumes the shared metadata via its own CPACK_* names.
if(NOT CMAKE_SYSTEM_NAME MATCHES "Windows")
    set(CPACK_STRIP_FILES TRUE)
    set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
    set(CPACK_PACKAGE_NAME "${PACKAGE_NAME}")
    set(CPACK_PACKAGE_VERSION "${PACKAGE_VERSION}")
    set(CPACK_PACKAGE_FILE_NAME "${PACKAGE_FILE_NAME}")
    INCLUDE (CPack)
endif()
