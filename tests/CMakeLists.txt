find_package(Catch2 CONFIG REQUIRED)

add_executable(run-tests
  test.cpp)

target_link_libraries(run-tests PRIVATE Catch2::Catch2 Catch2::Catch2WithMain)
target_link_libraries(run-tests PRIVATE array_list)

set_target_properties(run-tests
  PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF)

add_test(AllTestsInMain run-tests)
