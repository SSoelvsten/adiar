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
target_compile_definitions(test_unit PRIVATE COOM_DEBUG=0 COOM_ASSERT=1 COOM_PQ_BUCKETS=1)

# Link up directly to tpie for the tpie integration test
target_link_libraries(test_unit tpie)
