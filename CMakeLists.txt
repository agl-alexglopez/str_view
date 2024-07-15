cmake_minimum_required(VERSION 3.15)

set(namespace "str_view")
project("str_view"
    VERSION 0.1.0
    LANGUAGES C
    DESCRIPTION "Robust read-only string handling, tokenization, and matching in C."
)

# The goal of this section is to allow the tagged releases to run an
# automatic script that deletes the samples and test folders and any
# work that is intended for development only. The user should
# be able to download or clone the absolute bare minimum for this
# library to work.
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

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(${PROJECT_NAME} STATIC)

target_sources(${PROJECT_NAME}
    PRIVATE
        str_view/str_view.c
)

target_include_directories(${PROJECT_NAME}
    PRIVATE
        # where the library itself will look for its internal headers
        ${CMAKE_CURRENT_SOURCE_DIR}/str_view
    PUBLIC
        # where top-level project will look for the library's public headers
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        # where external projects will look for the library's public headers
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# totally optional listing of include directories
get_property(inclds
    TARGET ${PROJECT_NAME}
    PROPERTY INCLUDE_DIRECTORIES
)
message(STATUS "Listing include directories for ${PROJECT_NAME}...")
# "dumb" listing with "raw" generator expressions on configuration
foreach(pth ${inclds})
    message(STATUS "  ${pth}")
endforeach()
# actually evaluated generator expressions printed to file on generation
string(JOIN "\n" includeDirectories ${inclds})
file(GENERATE
    OUTPUT "${CMAKE_BINARY_DIR}/include-directories.txt"
    CONTENT ${includeDirectories}
)

# without it public headers won't get installed
set(public_headers
    str_view/str_view.h
)

# where to find our CMake modules
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
#message(STATUS "CMake module path: ${CMAKE_MODULE_PATH}")
include(Installing)
