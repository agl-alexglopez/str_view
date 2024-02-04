add_library(test INTERFACE test.h)

add_executable(run_tests run_tests.c)
target_link_libraries(run_tests str_view)

###########   Add Test Files Below This Point   ###########

add_executable(test_empty test_empty.c)
target_link_libraries(test_empty str_view)

add_executable(test_out_of_bounds test_out_of_bounds.c)
target_link_libraries(test_out_of_bounds str_view)


########   Put All Tests in One Folder   ##########
set_target_properties(
  test_empty
  test_out_of_bounds
  PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/tests
)
