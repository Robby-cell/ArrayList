enable_testing()

add_executable(test
  test.cpp)

find_package(Catch2 CONFIG REQUIRED)
target_link_libraries(test PRIVATE Catch2::Catch2 Catch2::Catch2WithMain)
target_link_libraries(test INTERFACE array_list_impl)

add_test(AllTestsInMain test)