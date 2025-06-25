# ============================================================================
# PROJECT SETUP
# ============================================================================
cmake_minimum_required(VERSION 3.15)
project(array_list VERSION 1.0.0 DESCRIPTION "A header-only array list library")

# It's good practice to set the C++ standard for your own project's targets (like tests)
# This is separate from the INTERFACE requirement for consumers, but keeps things consistent.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ============================================================================
# LIBRARY TARGET DEFINITION
# ============================================================================

add_library(array_list INTERFACE)

target_include_directories(array_list INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_compile_features(array_list INTERFACE cxx_std_17)


# ============================================================================
# INSTALLATION AND PACKAGING
# ============================================================================

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(TARGETS array_list
    EXPORT array_list_export_set
)

configure_package_config_file(
    "cmake/array_list-config.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake/array_list-config.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/cmake/array_list-config-version.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/cmake/array_list-config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake/array_list-config-version.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

install(EXPORT array_list_export_set
    FILE        ${PROJECT_NAME}-targets.cmake
    NAMESPACE   array_list::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

# ============================================================================
# TESTING SETUP (REFINED SECTION)
# ============================================================================

# 1. Provide a user-configurable option to build tests (defaults to ON).
#    This is better than `set()` because it can be changed from the command line:
#    cmake .. -DBUILD_TESTING=OFF
option(BUILD_TESTING "Build the project's tests" ON)

# 2. If tests are enabled, activate CTest and include the tests directory.
if(BUILD_TESTING)
    # This command is required to enable the `add_test` and `gtest_discover_tests`
    # commands and to allow `ctest` to run.
    enable_testing()
    add_subdirectory(tests)
endif()
