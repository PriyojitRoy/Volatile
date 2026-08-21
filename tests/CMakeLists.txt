set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
add_executable(run_movegen RunMoveGen.cpp)
# target_link_libraries(run_movegen PRIVATE EngineLib) # Uncomment when implemented
apply_hardware_optimizations(run_movegen)

add_executable(run_eval RunEval.cpp)
# target_link_libraries(run_eval PRIVATE EngineLib) # Uncomment when implemented
apply_hardware_optimizations(run_eval)

add_executable(run_sprt RunSPRT.cpp)
apply_hardware_optimizations(run_sprt)

add_executable(run_all_tests RunAllTests.cpp)
apply_hardware_optimizations(run_all_tests)
