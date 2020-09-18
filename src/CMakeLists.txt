add_subdirectory (coom)


# ============================================================================ #
# Setup DOT file printer
add_executable (coom_dot dot_main.cpp)
target_link_libraries(coom_dot tpie)
target_link_libraries(coom_dot coom)

target_compile_definitions(coom_dot PRIVATE COOM_DEBUG=0 COOM_ASSERT=0)

# ============================================================================ #
# Setup MAIN target
add_executable (coom_main main.cpp)
target_link_libraries(coom_main tpie)
target_link_libraries(coom_main coom)

target_compile_definitions(coom_main PRIVATE COOM_DEBUG=2 COOM_ASSERT=1)

