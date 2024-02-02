add_library(string_view string_view.h string_view.c)

add_subdirectory("util/")
target_include_directories(string_view PRIVATE "util/")

target_link_libraries(string_view PRIVATE sv_util)


