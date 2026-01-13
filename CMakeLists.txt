cmake_minimum_required(VERSION 3.14)
project(AdaptiveVolFramework VERSION 1.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Compiler warnings and optimizations for Release build
if(MSVC)
    add_compile_options(/W4 /permissive-)
else()
    add_compile_options(-Wall -Wextra -pedantic -O3 -march=native)
endif()

include(FetchContent)

# 1. Eigen (Math)
FetchContent_Declare(
  Eigen3
  URL https://gitlab.com/libeigen/eigen/-/archive/3.4.0/eigen-3.4.0.tar.gz
)
FetchContent_MakeAvailable(Eigen3)

# 2. GoogleTest (Testing)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Include directories
include_directories(include)

# Gather source files
file(GLOB_RECURSE SOURCES "src/*.cpp")
# Exclude main.cpp from the library, keep it for the executable
list(REMOVE_ITEM SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")

# --- Core Library ---
add_library(AdaptiveVolCore ${SOURCES})
target_link_libraries(AdaptiveVolCore PUBLIC Eigen3::Eigen)
target_include_directories(AdaptiveVolCore PUBLIC include)

# --- Main Demo Executable ---
add_executable(AdaptiveVolDemo src/main.cpp)
target_link_libraries(AdaptiveVolDemo PRIVATE AdaptiveVolCore)

# --- Unit Tests ---
enable_testing()

file(GLOB TEST_SOURCES "tests/*.cpp")
add_executable(UnitTests ${TEST_SOURCES})
target_link_libraries(UnitTests PRIVATE AdaptiveVolCore GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(UnitTests)