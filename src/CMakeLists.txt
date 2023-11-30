add_subdirectory (adiar)

# ============================================================================ #
# Setup MAIN target
add_executable (adiar_playground playground.cpp)
target_link_libraries(adiar_playground adiar)
