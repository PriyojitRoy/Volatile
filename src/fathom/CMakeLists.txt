target_sources(EngineLib PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/tbprobe.c
)

target_compile_options(EngineLib PRIVATE
    $<$<COMPILE_LANGUAGE:C>:-Wno-unused-parameter>
    $<$<COMPILE_LANGUAGE:C>:-Wno-unused-variable>
)
