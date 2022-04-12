
add_library (MODELS STATIC
    codeblock.cpp codeblock.h
    evidence.h
    tag.h
)

add_library(ASHIRT::MODELS ALIAS MODELS)

target_include_directories (MODELS
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries ( MODELS PUBLIC
    Qt::Core
)




