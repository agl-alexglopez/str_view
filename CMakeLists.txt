# File sets are available in 3.23 onward. Valuable tool.
cmake_minimum_required(VERSION 3.23)

set(namespace "str_view")
project("str_view"
    VERSION 0.5.5
    LANGUAGES C
    DESCRIPTION "Robust read-only string handling, tokenization, and matching in C."
)

# For the sake of vcpkg we will make sure cloning and building from
# main will only build the minimal files in the all target. I can't
# protect vcpkg from needlessly downloading 
if (EXISTS "${PROJECT_SOURCE_DIR}/samples")
    add_subdirectory("${PROJECT_SOURCE_DIR}/samples" EXCLUDE_FROM_ALL)
endif()
if (EXISTS "${PROJECT_SOURCE_DIR}/tests")
    add_subdirectory("${PROJECT_SOURCE_DIR}/tests" EXCLUDE_FROM_ALL)
endif()
if (EXISTS "${PROJECT_SOURCE_DIR}/tests" AND EXISTS "${PROJECT_SOURCE_DIR}/samples")
    include(etc/scanners.cmake)
endif()

if (NOT CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    message(STATUS "This project has a top-level one called [${CMAKE_PROJECT_NAME}]")
else()
    message(STATUS "This project is a top-level one")
endif()

option(BUILD_SHARED_LIBS "Building str_view as a shared library is available, but OFF by default." OFF)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/debug/bin)
else()
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
endif()


add_subdirectory("${PROJECT_SOURCE_DIR}/${PROJECT_NAME}")

# where to find our CMake modules
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
#message(STATUS "CMake module path: ${CMAKE_MODULE_PATH}")
include(Installing)
