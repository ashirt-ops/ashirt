
add_library (DB STATIC
    databaseconnection.cpp
    databaseconnection.h
    query_result.h
    ${CMAKE_SOURCE_DIR}/migrations/res_migrations.qrc
)

add_library(ASHIRT::DB ALIAS DB)

target_include_directories (DB
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries ( DB PUBLIC
    Qt::Sql
)
