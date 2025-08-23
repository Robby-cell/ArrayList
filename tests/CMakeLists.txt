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

add_library(c++11-test STATIC
  c++11.cpp
  c++11.hpp)
target_link_libraries(c++11-test PRIVATE Catch2::Catch2)
target_link_libraries(c++11-test PRIVATE array_list)

set_target_properties(c++11-test
  PROPERTIES
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF)

target_link_libraries(run-tests PRIVATE c++11-test)

add_test(AllTestsInMain run-tests)
