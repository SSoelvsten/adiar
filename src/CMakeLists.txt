add_subdirectory (coom)


# ============================================================================ #
# Setup DOT file printer
add_executable (coom_dot dot_main.cpp)
target_link_libraries(coom_dot coom)

# ============================================================================ #
# Setup MAIN target
add_executable (coom_main main.cpp)
target_link_libraries(coom_main coom)
