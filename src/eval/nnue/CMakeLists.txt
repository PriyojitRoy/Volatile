# Accumulator is always compiled: Board struct embeds it regardless of eval backend
target_sources(EngineLib PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/Accumulator.cpp
)

# Network.cpp (NNUE evaluator stub) is only compiled when USE_NNUE is ON
if(USE_NNUE)
    target_sources(EngineLib PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Network.cpp
    )
endif()
