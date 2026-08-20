cmake_minimum_required(VERSION 3.13)
project(Volatile LANGUAGES CXX C)

# ==============================================================================
# Build Configuration Options
# ==============================================================================
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(USE_NNUE "Build with NNUE evaluation instead of HCE" OFF)
if(USE_NNUE)
    message(STATUS "Evaluation backend: NNUE")
else()
    message(STATUS "Evaluation backend: HCE (default)")
endif()

# ==============================================================================
# Architecture & Warning Optimizations
# ==============================================================================
set(GCC_CLANG_WARNINGS
    -Wall
    -Wextra
    -Wno-unused-parameter
    -Wno-unused-variable
    -Wno-array-bounds
)

set(MSVC_WARNINGS
    /W4
    /wd4101 # unreferenced local variable
    /wd4100 # unreferenced formal parameter
)

function(apply_hardware_optimizations target_name)
    if(MSVC)
        target_compile_options(${target_name} PUBLIC ${MSVC_WARNINGS} /Ox /DNDEBUG)
        if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64" OR CMAKE_SYSTEM_PROCESSOR MATCHES "amd64")
            target_compile_options(${target_name} PUBLIC /arch:AVX2)
            message(STATUS "Detected x86_64 (MSVC) - Enabling /arch:AVX2")
        endif()
    else()
        target_compile_options(${target_name} PUBLIC ${GCC_CLANG_WARNINGS} -O3 -DNDEBUG)
        if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64" OR CMAKE_SYSTEM_PROCESSOR MATCHES "amd64")
            target_compile_options(${target_name} PUBLIC -march=native -mbmi2 -mavx2)
            message(STATUS "Detected x86_64 (GCC/Clang) - Enabling AVX2 and BMI2")
        elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64" OR CMAKE_SYSTEM_PROCESSOR MATCHES "arm")
            target_compile_options(${target_name} PUBLIC -march=native)
            message(STATUS "Detected ARM64 (GCC/Clang) - Enabling NEON optimizations")
        endif()
    endif()
endfunction()

# ==============================================================================
# Global Optimization: Link-Time Optimization (LTO / IPO)
# ==============================================================================
# Force Termux (Android) Clang to use 'lld' linker for successful LTO compilation
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND CMAKE_SYSTEM_NAME MATCHES "Android")
    add_link_options(-fuse-ld=lld)
endif()

cmake_policy(SET CMP0069 NEW)
include(CheckIPOSupported)
check_ipo_supported(RESULT ipo_supported OUTPUT error)

if(ipo_supported)
    message(STATUS "IPO / LTO supported - Enabling it for maximum speed!")
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
else()
    message(STATUS "IPO / LTO not supported on this platform/compiler.")
endif()

# ==============================================================================
# Target Definitions
# ==============================================================================
# 1. Engine Library (Core logic)
add_library(EngineLib STATIC)
target_include_directories(EngineLib PUBLIC include)
apply_hardware_optimizations(EngineLib)

if(USE_NNUE)
    target_compile_definitions(EngineLib PUBLIC USE_NNUE)
endif()

add_subdirectory(src)

# 2. Main Executable (CLI/UCI Interface)
add_executable(Volatile src/main.cpp)
target_link_libraries(Volatile PRIVATE EngineLib)

# ==============================================================================
# Testing Suite
# ==============================================================================
enable_testing()

macro(add_volatile_test TEST_NAME SOURCE_FILE)
    add_executable(${TEST_NAME} ${SOURCE_FILE})
    target_link_libraries(${TEST_NAME} PRIVATE EngineLib)
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
endmacro()

add_volatile_test(test_board tests/TestBoard.cpp)
add_volatile_test(test_movegen tests/TestMoveGen.cpp)