macro(add_test NAME SOURCE)
  add_executable (test-${NAME} ${SOURCE})

  # Parse CMake options to executable
  # TODO

  # Link up with Bandit
  set(BANDIT_FLAGS "--reporter=info --colorizer=light")
  target_include_directories (test-${NAME} PRIVATE ${PROJECT_SOURCE_DIR}/external)

  # Link with Adiar
  target_link_libraries(test-${NAME} adiar)
endmacro(add_test)

# ============================================================================ #
# Add 'test-adiar' target
add_test(adiar test.cpp)
target_compile_definitions(test-adiar PRIVATE ADIAR_TEST_PRINT_STATS)

# ============================================================================ #
# Add individual test files
add_subdirectory (adiar)
