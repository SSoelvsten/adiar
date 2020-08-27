# ============================================================================ #
# N-Queens example
add_executable (n_queens n_queens.cpp)

target_link_libraries(n_queens coom)
target_link_libraries(n_queens tpie)

# ============================================================================ #
# Tic-Tac-Toe example
add_executable (tic_tac_toe tic_tac_toe.cpp)

target_link_libraries(tic_tac_toe coom)
target_link_libraries(tic_tac_toe tpie)
