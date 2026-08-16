target_sources(EngineLib PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/Bitboard.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ConstantArrays.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Move.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/MoveGen.cpp
)

if(USE_NNUE)
    add_subdirectory(nnue)
else()
    add_subdirectory(hce)
endif()
