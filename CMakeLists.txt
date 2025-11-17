cmake_minimum_required(VERSION 3.14)
project(AntColonyTSP VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# OpenMP support (optional but recommended for parallel performance)
find_package(OpenMP)
if(OpenMP_CXX_FOUND)
    message(STATUS "OpenMP found - parallel execution enabled")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
else()
    message(WARNING "OpenMP not found - will use serial execution")
endif()

# Generate compile_commands.json for clang tooling (clangd, etc.)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Compiler warnings
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Include directories
include_directories(${PROJECT_SOURCE_DIR}/include)

# Collect source files
file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "include/*.h")

# Main executable
add_executable(ant_colony_tsp ${SOURCES} ${HEADERS})

target_include_directories(ant_colony_tsp
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

# Link OpenMP if found
if(OpenMP_CXX_FOUND)
    target_link_libraries(ant_colony_tsp OpenMP::OpenMP_CXX)
endif()

# ============================================================================
# Google Test Setup
# ============================================================================

include(FetchContent)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

# Enable testing
enable_testing()

# Collect test files
file(GLOB_RECURSE TEST_SOURCES "tests/*.cpp")

# Create test executable
if(TEST_SOURCES)
    # Exclude main.cpp from library sources for testing
    list(FILTER SOURCES EXCLUDE REGEX ".*main\\.cpp$")

    add_executable(ant_colony_tests ${TEST_SOURCES} ${SOURCES})

    target_include_directories(ant_colony_tests
        PUBLIC
            ${PROJECT_SOURCE_DIR}/include
    )

    target_link_libraries(ant_colony_tests
        gtest
        gtest_main
        gmock
    )

    # Link OpenMP to tests if found
    if(OpenMP_CXX_FOUND)
        target_link_libraries(ant_colony_tests OpenMP::OpenMP_CXX)
    endif()

    # Discover tests for CTest
    include(GoogleTest)
    gtest_discover_tests(ant_colony_tests)
endif()

# ============================================================================
# Installation
# ============================================================================

install(TARGETS ant_colony_tsp
    RUNTIME DESTINATION bin
)

install(DIRECTORY include/
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)
