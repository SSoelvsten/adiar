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
  VERSION 1.0.0
  DESCRIPTION "Adiar, an external memory decision diagram library"
  HOMEPAGE_URL "https://ssoelvsten.github.io/adiar/"
  LANGUAGES CXX
)

enable_language(CXX)

message(STATUS "CMake build configuration for Adiar ${PROJECT_VERSION}")
message(STATUS "  OS: ${CMAKE_SYSTEM_NAME}")
message(STATUS "  Compiler: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "    C Flags: ${CMAKE_C_FLAGS}")
message(STATUS "    CXX Flags: ${CMAKE_CXX_FLAGS}")
message(STATUS "    EXE Linker Flags: ${CMAKE_EXE_LINKER_FLAGS}")
message(STATUS "  Tests and Examples: ${MAIN_PROJECT}")
message(STATUS "")

# ============================================================================ #
# Dependencies
# ============================================================================ #
add_subdirectory (external/tpie tpie)

# ============================================================================ #
# Core project
# ============================================================================ #
option(ADIAR_SHARED "Build adiar as a shared library" OFF)
message(STATUS "  Shared library: ${ADIAR_SHARED}")

add_subdirectory (src)

install(DIRECTORY src/adiar
        DESTINATION include
        FILES_MATCHING REGEX "\\.h$")

# ============================================================================ #
# Unit Tests
# ============================================================================ #
if (MAIN_PROJECT)
  add_subdirectory (test)
endif()

# ============================================================================ #
# Examples
# ============================================================================ #
if (MAIN_PROJECT)
  add_subdirectory (example)
endif ()
