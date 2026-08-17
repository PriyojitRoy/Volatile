cmake_minimum_required(VERSION 3.10)
project(Volatile LANGUAGES CXX C)

# ==============================================================================
# Core Project Settings
# ==============================================================================
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ==============================================================================
# Build Options (HCE vs NNUE)
# ==============================================================================
option(USE_NNUE "Build with NNUE evaluation instead of HCE" OFF)

if(USE_NNUE)
    message(STATUS "Evaluation backend: NNUE")
else()
    message(STATUS "Evaluation backend: HCE (default)")
endif()

# ==============================================================================
# Compiler Warnings & Base Optimizations
# ==============================================================================
if(MSVC)
    add_compile_options(/W4 /Ox /DNDEBUG)
else()
    add_compile_options(-Wall -Wextra -O3 -DNDEBUG)
endif()

# ==============================================================================
# Architecture-Specific Hardware Optimizations (AVX2, BMI2, NEON)
# ==============================================================================
if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64" OR CMAKE_SYSTEM_PROCESSOR MATCHES "amd64")
    if(MSVC)
        message(STATUS "Detected x86_64 (MSVC) - Enabling /arch:AVX2")
        add_compile_options(/arch:AVX2)
    else()
        message(STATUS "Detected x86_64 (GCC/Clang) - Enabling AVX2 and BMI2")
        add_compile_options(-march=native -mbmi2 -mavx2)
    endif()

elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64" OR CMAKE_SYSTEM_PROCESSOR MATCHES "arm")
    if(NOT MSVC)
        message(STATUS "Detected ARM64 (GCC/Clang) - Enabling NEON optimizations")
        add_compile_options(-march=native)
    endif()
endif()

# ==============================================================================
# Link-Time Optimization (LTO / IPO)
# ==============================================================================
cmake_policy(SET CMP0069 NEW)
include(CheckIPOSupported)
check_ipo_supported(RESULT ipo_supported OUTPUT error)

if(ipo_supported)
    message(STATUS "IPO / LTO supported - Enabling it for maximum speed!")
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
else()
    message(STATUS "IPO / LTO not supported: <${error}>")
endif()

# ==============================================================================
# Target Library (EngineLib)
# ==============================================================================
# Create the static library target; sources are populated by subdirectory CMakeLists
add_library(EngineLib STATIC)

# Modern CMake: Scope includes and definitions exclusively to the target
target_include_directories(EngineLib PUBLIC include)

if(USE_NNUE)
    target_compile_definitions(EngineLib PUBLIC USE_NNUE)
endif()

# Load subdirectories
add_subdirectory(src)

# ==============================================================================
# Main Executable
# ==============================================================================
add_executable(Volatile src/main.cpp)
target_link_libraries(Volatile PRIVATE EngineLib)

# ==============================================================================
# Testing Environment
# ==============================================================================
enable_testing()

# Reusable macro to easily add new test binaries
macro(add_volatile_test TEST_NAME SOURCE_FILE)
    add_executable(${TEST_NAME} ${SOURCE_FILE})
    target_link_libraries(${TEST_NAME} PRIVATE EngineLib)
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
endmacro()

# Register current tests
add_volatile_test(test_board tests/TestBoard.cpp)
add_volatile_test(test_movegen tests/TestMoveGen.cpp)