# ============================================================================ #
# N-Queens example
add_executable (queens queens.cpp)

target_link_libraries(queens adiar)

# ============================================================================ #
# N-Queens example
add_executable (knights_tour knights_tour_bryant.cpp)

target_link_libraries(knights_tour adiar)
