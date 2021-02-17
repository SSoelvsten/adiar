# ============================================================================ #
# N-Queens example
add_executable (queens queens.cpp)

target_link_libraries(queens adiar)

# ============================================================================ #
# Pigeonhole Principle example
add_executable (pigeonhole_principle pigeonhole_principle.cpp)

target_link_libraries(pigeonhole_principle adiar)

# ============================================================================ #
# Tic-Tac-Toe example
add_executable (tic_tac_toe tic_tac_toe.cpp)

target_link_libraries(tic_tac_toe adiar)
