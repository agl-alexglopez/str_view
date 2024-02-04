add_library(str_view str_view.h str_view.c)

add_subdirectory("util/")
target_include_directories(str_view PRIVATE "util/")

target_link_libraries(str_view PRIVATE sv_util)


