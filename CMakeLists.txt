cmake_minimum_required(VERSION 3.14)
project(c-str-view VERSION 0.1.0 LANGUAGES C)

include(etc/scanners.cmake)

include_directories("${PROJECT_SOURCE_DIR}/sammples")
include_directories("${PROJECT_SOURCE_DIR}/str_view")
include_directories("${PROJECT_SOURCE_DIR}/tests")

add_subdirectory("${PROJECT_SOURCE_DIR}/samples")
add_subdirectory("${PROJECT_SOURCE_DIR}/str_view")
add_subdirectory("${PROJECT_SOURCE_DIR}/tests")
