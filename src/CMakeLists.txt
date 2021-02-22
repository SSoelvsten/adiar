add_subdirectory (adiar)

# ============================================================================ #
# Setup MAIN target
add_executable (adiar_main main.cpp)
target_link_libraries(adiar_main adiar)
