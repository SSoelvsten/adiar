# Parts of this CMakeLists.txt is derived from the files of the tpie, lace and
# sylvan projects.

# Check if this is used as part of a solo project.
if(DEFINED PROJECT_NAME)
  set(MAIN_PROJECT OFF)
else()
  set(MAIN_PROJECT ON)
endif()

cmake_minimum_required(VERSION 3.10)
project (adiar
  VERSION 1.2.1
  DESCRIPTION "Adiar, an external memory decision diagram library"
  HOMEPAGE_URL "https://ssoelvsten.github.io/adiar/"
  LANGUAGES CXX
)

enable_language(CXX)

# ============================================================================ #
# Settings
# ============================================================================ #

message(STATUS "CMake build configuration for Adiar ${PROJECT_VERSION}")

option(ADIAR_STATS_EXTRA "Collect statistics (deprecated)" OFF)
if (ADIAR_STATS_EXTRA)
  message(DEPRECATED "  The 'ADIAR_STATS_EXTRA' variable has been merged with 'ADIAR_STATS'.\n")
  set(ADIAR_STATS ON)
endif (ADIAR_STATS_EXTRA)

message(STATUS "  OS: ${CMAKE_SYSTEM_NAME}")
message(STATUS "  Compiler: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "    C Flags: ${CMAKE_C_FLAGS}")
message(STATUS "    CXX Flags: ${CMAKE_CXX_FLAGS}")
message(STATUS "    EXE Linker Flags: ${CMAKE_EXE_LINKER_FLAGS}")

option(ADIAR_SHARED "Build adiar as a shared library" OFF)
message(STATUS "  Shared: ${ADIAR_SHARED}")

option(ADIAR_STATS "Collect statistics" OFF)

message(STATUS "  Statistics: ${ADIAR_STATS}")

message(STATUS "  Optional targets:")

option(ADIAR_TEST "Build unit tests for adiar" ${MAIN_PROJECT})
message(STATUS "    Unit Tests: ${MAIN_PROJECT}")

option(ADIAR_EXAMPLES "Build examples for usage of adiar" ${MAIN_PROJECT})
message(STATUS "    Examples: ${MAIN_PROJECT}")

message(STATUS "")

# ============================================================================ #
# Dependencies
# ============================================================================ #
option(COMPILE_TEST OFF)
add_subdirectory (external/tpie tpie)

add_subdirectory (external/cnl cnl)

# ============================================================================ #
# Core project
# ============================================================================ #
add_subdirectory (src)

install(DIRECTORY src/adiar
        DESTINATION include
        FILES_MATCHING REGEX "\\.h$")

# ============================================================================ #
# Documentation
# ============================================================================ #
option(ADIAR_DOCS "Build Documentation for Adiar" ON)

if (ADIAR_DOCS)
  add_subdirectory(docs)
endif (AIDAR_DOCS)

# ============================================================================ #
# Unit Tests
# ============================================================================ #
if (ADIAR_TEST)
  add_subdirectory (test)
endif()

# ============================================================================ #
# Examples
# ============================================================================ #
if (ADIAR_EXAMPLES)
  add_subdirectory (example)
endif ()
