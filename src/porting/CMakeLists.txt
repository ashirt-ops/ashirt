
add_library (PORTING STATIC
    evidence_manifest.h
    system_manifest.cpp system_manifest.h
    system_porting_options.h
)

add_library(ASHIRT::PORTING ALIAS PORTING)

target_include_directories (PORTING
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries ( PORTING PUBLIC
    ASHIRT::DB
    ASHIRT::MODELS
)




