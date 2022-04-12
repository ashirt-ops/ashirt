
add_library (EXCEPTIONS STATIC
    databaseerr.h
    fileerror.h
)

add_library(ASHIRT::EXCEPTIONS ALIAS EXCEPTIONS)

target_include_directories (EXCEPTIONS
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

target_link_libraries ( EXCEPTIONS PUBLIC
    Qt::Core
)


