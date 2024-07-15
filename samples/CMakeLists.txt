cmake_minimum_required(VERSION 3.15)
project("str_view samples"
    VERSION 0.1.0
    LANGUAGES C
    DESCRIPTION "Testing the str_view library with sample programs."
)

include_directories("../str_view")
add_executable(mini_grep mini_grep.c)
target_link_libraries(mini_grep str_view)

add_custom_target(samples)
# Function to add dependencies on all targets from the subdirectory
# Get the list of all targets
get_property(sample_targets DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY BUILDSYSTEM_TARGETS)
foreach(target IN LISTS sample_targets)
    add_dependencies(samples ${target})
endforeach()

