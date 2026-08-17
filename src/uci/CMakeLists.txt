target_sources(EngineLib PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/UCIHandler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/commands/CommandGo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/commands/CommandPosition.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/commands/CommandPlay.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/commands/CommandSetOption.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/commands/CommandPerft.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/commands/CommandDisplay.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/commands/CommandSysinfo.cpp
)
