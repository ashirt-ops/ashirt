
add_library (DTOS STATIC
    checkConnection.h
    github_release.h
    operation.h
    tag.h
)

add_library(ASHIRT::DTOS ALIAS DTOS)

target_include_directories (DTOS
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

target_link_libraries ( DTOS PUBLIC
    ASHIRT::MODELS
)


