# ============================================================================ #
# Add main Bandit test file
add_executable (test_unit test.cpp)

# ============================================================================ #
# Link up with Bandit
set(BANDIT_FLAGS "--reporter=info --colorizer=light")
target_include_directories (test_unit
PRIVATE
  ${PROJECT_SOURCE_DIR}/external
)

# Link up with coom source
target_link_libraries(test_unit coom)
target_link_libraries(test_unit tpie)
