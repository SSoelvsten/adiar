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
  VERSION 1.1.0
  DESCRIPTION "Adiar, an external memory decision diagram library"
  HOMEPAGE_URL "https://ssoelvsten.github.io/adiar/"
  LANGUAGES CXX
)

enable_language(CXX)

# ============================================================================ #
# Settings
# ============================================================================ #

message(STATUS "CMake build configuration for Adiar ${PROJECT_VERSION}")
message(STATUS "  OS: ${CMAKE_SYSTEM_NAME}")
message(STATUS "  Compiler: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "    C Flags: ${CMAKE_C_FLAGS}")
message(STATUS "    CXX Flags: ${CMAKE_CXX_FLAGS}")
message(STATUS "    EXE Linker Flags: ${CMAKE_EXE_LINKER_FLAGS}")

option(ADIAR_SHARED "Build adiar as a shared library" OFF)
message(STATUS "  Shared: ${ADIAR_SHARED}")

message(STATUS "  Statistics:")

option(ADIAR_STATS "Collect statistics" OFF)

option(ADIAR_STATS_EXTRA "Collect (a lot of) statistics" OFF)
if (ADIAR_STATS_EXTRA)
  set(ADIAR_STATS ON)
endif (ADIAR_STATS_EXTRA)

message(STATUS "    Coarse: ${ADIAR_STATS}")
message(STATUS "    Fine-grained: ${ADIAR_STATS_EXTRA}")

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
