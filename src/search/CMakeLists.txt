target_sources(EngineLib PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/OpeningBook.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/TT.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Tablebase.cpp
)

if(USE_NNUE)
    add_subdirectory(nnue)
else()
    add_subdirectory(hce)
endif()
