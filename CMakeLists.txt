cmake_minimum_required(VERSION 3.18)

project(ashirt VERSION 1.2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(NOTARIZE_AS "" CACHE STRING "Attempt to Sign Package With Provided User")
if(EXISTS ${CMAKE_SOURCE_DIR}/.git)
    find_package(Git)
    if(GIT_FOUND)
     EXECUTE_PROCESS(
      COMMAND ${GIT_EXECUTABLE} describe --long --match v*
      WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
      OUTPUT_VARIABLE GITREV
      ERROR_QUIET
      OUTPUT_STRIP_TRAILING_WHITESPACE)
     string(REGEX MATCH [0-9]+ MAJOR ${GITREV})
     string(REGEX MATCH \\.[0-9]+ MINOR ${GITREV})
     string(REPLACE "." "" MINOR ${MINOR})
     string(REGEX MATCH [0-9]+\- PATCH ${GITREV})
     string(REPLACE "-" "" PATCH ${PATCH})
     string(REGEX MATCH \-[0-9]+\- TWEAK ${GITREV})
     string(REPLACE "-" "" TWEAK ${TWEAK})
     set(CMAKE_PROJECT_VERSION_MAJOR ${MAJOR})
     set(CMAKE_PROJECT_VERSION_MINOR ${MINOR})
     set(CMAKE_PROJECT_VERSION_PATCH ${PATCH})
     set(CMAKE_PROJECT_VERSION_TWEAK ${TWEAK})
     set(CMAKE_PROJECT_VERSION "${MAJOR}.${MINOR}.${PATCH}.${TWEAK}")
     set(VERSION_TAG "v${MAJOR}.${MINOR}.${PATCH}")
     EXECUTE_PROCESS(
      COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
      WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
      OUTPUT_VARIABLE COMMIT_HASH
      ERROR_QUIET
      OUTPUT_STRIP_TRAILING_WHITESPACE)
    EXECUTE_PROCESS(
      COMMAND ${GIT_EXECUTABLE} remote get-url origin
      WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
      OUTPUT_VARIABLE GITORIGIN
      ERROR_QUIET
      OUTPUT_STRIP_TRAILING_WHITESPACE)
    string(REPLACE "https://github.com/" ":" GITORIGIN ${GITORIGIN})
    string(REPLACE "git@github.com" "" GITORIGIN ${GITORIGIN})
    string(REGEX MATCH [[:[A-Za-z0-9]+]] REPO_OWNER ${GITORIGIN})
    string(REPLACE ":" "" REPO_OWNER ${REPO_OWNER})
    string(REGEX MATCH [[/[A-Za-z0-9]+]] REPO_NAME ${GITORIGIN})
    string(REPLACE "/" "" REPO_NAME ${REPO_NAME})
    endif()
else()
    set(COMMIT_HASH "")
    set(REPO_NAME "ashirt")
    set(REPO_OWNER "theparanoids")
    set(VERSION_TAG "v0.0.0")
endif()
     message(STATUS "VERSION: ${CMAKE_PROJECT_VERSION}")

find_package(Qt6 6.5.0 REQUIRED COMPONENTS
    Widgets
    Gui
    Network
    Sql
    Core
    Svg
)

add_subdirectory(deploy)
add_subdirectory(src)
