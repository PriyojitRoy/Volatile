target_sources(EngineLib PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/tbprobe.c
)

set(FATHOM_SUPPRESSED_WARNINGS
    -Wno-unused-parameter
    -Wno-unused-variable
    -Wno-array-bounds
)

target_compile_options(EngineLib PRIVATE
    $<$<COMPILE_LANGUAGE:C>:${FATHOM_SUPPRESSED_WARNINGS}>
)
