cmake_minimum_required(VERSION 3.15)
project(fft-dsp-toolkit 
    VERSION 1.0.0 
    DESCRIPTION "Production-grade FFT and DSP toolkit"
    LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Werror -fPIC")
set(CMAKE_C_FLAGS_DEBUG "-g -O0 -DDEBUG")
set(CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG")

# Default to Release build
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Enable testing
enable_testing()

# Include directories
include_directories(include)

# Core library sources
set(DSP_SOURCES
    src/fft.c
    src/filter.c
    src/dsp_utils.c
)

# Create shared library
add_library(fft_dsp_shared SHARED ${DSP_SOURCES})
set_target_properties(fft_dsp_shared PROPERTIES
    OUTPUT_NAME fft_dsp
    VERSION 1.0.0
    SOVERSION 1
)

# Create static library
add_library(fft_dsp_static STATIC ${DSP_SOURCES})
set_target_properties(fft_dsp_static PROPERTIES
    OUTPUT_NAME fft_dsp
)

# Examples
add_executable(fft_demo examples/fft_demo.c)
target_link_libraries(fft_demo fft_dsp_static m)

add_executable(filter_demo examples/filter_demo.c)
target_link_libraries(filter_demo fft_dsp_static m)

# Unit Tests (minimal framework, no external dependencies)
add_executable(test_fft tests/test_fft.c)
target_include_directories(test_fft PRIVATE tests)
target_link_libraries(test_fft fft_dsp_static m)
add_test(NAME fft_tests COMMAND test_fft)

add_executable(test_filter tests/test_filter.c)
target_include_directories(test_filter PRIVATE tests)
target_link_libraries(test_filter fft_dsp_static m)
add_test(NAME filter_tests COMMAND test_filter)

# Build output directories
set_target_properties(
    fft_demo filter_demo test_fft test_filter
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

set_target_properties(
    fft_dsp_shared fft_dsp_static
    PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)

# Installation
install(TARGETS fft_dsp_shared fft_dsp_static
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)
install(DIRECTORY include/ DESTINATION include)

# Formatting target (clang-format)
add_custom_target(format
    COMMAND clang-format -i src/*.c include/*.h examples/*.c tests/test_*.c
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

# Code coverage target
if(ENABLE_COVERAGE)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --coverage")
    add_custom_target(coverage
        COMMAND ctest
        COMMAND lcov --directory . --capture --output-file coverage.info
        COMMAND lcov --remove coverage.info '/usr/*' '*/tests/*' --output-file coverage.info
        COMMAND genhtml coverage.info --output-directory coverage_html
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
endif()

# Print configuration
message(STATUS "=== FFT-DSP Toolkit Configuration ===")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C Compiler: ${CMAKE_C_COMPILER}")
message(STATUS "C Flags: ${CMAKE_C_FLAGS}")
message(STATUS "C++ Flags: ${CMAKE_C_FLAGS_${CMAKE_BUILD_TYPE}}")
message(STATUS "====================================")
