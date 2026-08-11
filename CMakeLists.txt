cmake_minimum_required(VERSION 3.10)
project(Volatile LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

cmake_policy(SET CMP0069 NEW)
include(CheckIPOSupported)
check_ipo_supported(RESULT ipo_supported OUTPUT error)
if(ipo_supported)
    message(STATUS "IPO / LTO supported - Enabling it for maximum speed!")
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
else()
    message(STATUS "IPO / LTO not supported: <${error}>")
endif()

add_compile_definitions(NDEBUG)

if(MSVC)
    add_compile_options(/W4 /Ox)
else()
    add_compile_options(-Wall -Wextra -O3)
endif()

if(NOT MSVC)
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64" OR CMAKE_SYSTEM_PROCESSOR MATCHES "amd64")
        message(STATUS "Detected x86_64 - Enabling AVX2 and BMI2")
        add_compile_options(-march=native -mbmi2 -mavx2)
        
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64" OR CMAKE_SYSTEM_PROCESSOR MATCHES "arm")
        message(STATUS "Detected ARM64 (Termux) - Enabling NEON optimizations")
        add_compile_options(-march=native)
    endif()
endif()

# ---------------------------------------------------------------------------
# Evaluation backend selection
#   cmake -DUSE_NNUE=OFF ..   -> HCE evaluation (default)
#   cmake -DUSE_NNUE=ON  ..   -> NNUE evaluation (stub, work in progress)
# ---------------------------------------------------------------------------
option(USE_NNUE "Build with NNUE evaluation instead of HCE" OFF)
if(USE_NNUE)
    message(STATUS "Evaluation backend: NNUE")
    add_compile_definitions(USE_NNUE)
else()
    message(STATUS "Evaluation backend: HCE (default)")
endif()

include_directories(include)

# Create the static library target; sources are populated by subdirectory CMakeLists files
add_library(EngineLib STATIC)

add_subdirectory(src)

add_executable(Volatile src/main.cpp)
target_link_libraries(Volatile EngineLib)

enable_testing()

add_executable(test_board tests/TestBoard.cpp)
target_link_libraries(test_board EngineLib)
add_test(NAME BoardTests COMMAND test_board)

add_executable(test_movegen tests/TestMoveGen.cpp)
target_link_libraries(test_movegen EngineLib)
add_test(NAME MoveGenTests COMMAND test_movegen)