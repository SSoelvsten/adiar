cmake_minimum_required(VERSION 3.21)
cmake_policy(SET CMP0048 NEW)
cmake_policy(SET CMP0063 NEW)

# Parts of this CMakeLists.txt is derived from the files of the tpie, lace and
# sylvan projects.

project (adiar
  VERSION 2.0.0
  DESCRIPTION "Adiar, an external memory decision diagram library"
  HOMEPAGE_URL "https://ssoelvsten.github.io/adiar/"
  LANGUAGES CXX
)

enable_language(CXX)

# ============================================================================ #
# Settings
# ============================================================================ #

message(STATUS "CMake build configuration for Adiar ${PROJECT_VERSION}")

message(STATUS "  OS:                        ${CMAKE_SYSTEM_NAME}")
message(STATUS "  Compiler:                  ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(VERBOSE "    C Flags:                ${CMAKE_C_FLAGS}")
message(VERBOSE "    CXX Flags:              ${CMAKE_CXX_FLAGS}")
message(VERBOSE "    EXE Linker Flags:       ${CMAKE_EXE_LINKER_FLAGS}")

message(STATUS "  Library Options:")

option(ADIAR_SHARED "Build adiar as a shared library" OFF)
message(STATUS "  | Shared:                  ${ADIAR_SHARED}")

option(ADIAR_STATS "Collect statistics" OFF)
message(STATUS "  | Statistics:              ${ADIAR_STATS}")

message(STATUS "  Optional targets:")

option(ADIAR_DOCS "Build Documentation for Adiar" ON)
message(STATUS "  | Documentation (Doxygen): ${ADIAR_DOCS}")

option(ADIAR_TEST "Build unit tests for adiar" ${PROJECT_IS_TOP_LEVEL})
message(STATUS "  | Unit Tests:              ${ADIAR_TEST}")

option(ADIAR_EXAMPLES "Build examples for usage of adiar" ${PROJECT_IS_TOP_LEVEL})
message(STATUS "  | Examples:                ${ADIAR_EXAMPLES}")

message(STATUS "")

if (ADIAR_SHARED)
  message (FATAL_ERROR "Adiar's CMake files do not (yet) support building Adiar as a shared library (GitHuB Issue #200). Any help to do so, would be very much appreciated.")
endif (ADIAR_SHARED)

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

# ============================================================================ #
# Documentation
# ============================================================================ #
if (ADIAR_DOCS)
  add_subdirectory(docs)
endif (ADIAR_DOCS)

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
