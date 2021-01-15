add_subdirectory (adiar)


# ============================================================================ #
# Setup DOT file printer
add_executable (adiar_dot dot_main.cpp)
target_link_libraries(adiar_dot adiar)

# ============================================================================ #
# Setup MAIN target
add_executable (adiar_main main.cpp)
target_link_libraries(adiar_main adiar)
