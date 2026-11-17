cmake_minimum_required(VERSION 3.20)
project(tagstream LANGUAGES CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Your code, as a library both the app and tests use
add_library(tagstream_lib src/matcher.cpp)
target_include_directories(tagstream_lib PUBLIC include)
target_compile_options(tagstream_lib PRIVATE -Wall -Wextra)

# The command-line program
add_executable(tagstream src/main.cpp)
target_link_libraries(tagstream PRIVATE tagstream_lib)

# Download GoogleTest
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.zip
)
FetchContent_MakeAvailable(googletest)

# The test program
enable_testing()
add_executable(tagstream_tests tests/matcher_test.cpp)
target_link_libraries(tagstream_tests PRIVATE tagstream_lib GTest::gtest_main)
include(GoogleTest)
gtest_discover_tests(tagstream_tests)