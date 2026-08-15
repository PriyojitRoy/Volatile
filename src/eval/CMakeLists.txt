# nnue subdirectory always included: Accumulator.cpp is needed by Board regardless of eval backend.
# Network.cpp is conditionally included inside the nnue subdirectory (see src/eval/nnue/CMakeLists.txt).
add_subdirectory(nnue)

# Add the selected evaluation backend implementation
if(USE_NNUE)
    # Network.cpp already handled in nnue subdirectory above
    message(STATUS "  -> Compiling NNUE evaluation backend (Network.cpp)")
else()
    add_subdirectory(hce)
    message(STATUS "  -> Compiling HCE evaluation backend (Evaluate.cpp)")
endif()
