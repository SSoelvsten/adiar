# Parts of this CMakeLists.txt is derived from the files of the tpie and
# sylvan projects.

cmake_minimum_required(VERSION 3.8)
project (adiar
  VERSION 0.2
  DESCRIPTION "Adiar, an external memory decision diagram library"
  LANGUAGES CXX
)

message(STATUS "CMake build configuration for Adiar ${PROJECT_VERSION}")
enable_language(CXX)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_FLAGS "-O2 ${CMAKE_C_FLAGS}")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "-O2 ${CMAKE_CXX_FLAGS}")

# ============================================================================ #
# Common Dependencies
# ============================================================================ #
add_subdirectory (external/tpie tpie)

# ============================================================================ #
# Core project
# ============================================================================ #
option(ADIAR_SHARED "Build adiar as a shared library" OFF)

add_subdirectory (src)

# ============================================================================ #
# Test Files
# ============================================================================ #
option(COMPILE_TEST "Build adiar unit tests" ON)
if (COMPILE_TEST)
  # enable_testing() # No need, as we use Bandit
  add_subdirectory (test)
endif()

# ============================================================================ #
# install Adiar as a library to use
# ============================================================================ #
install(DIRECTORY src/adiar
        DESTINATION include
        FILES_MATCHING REGEX "\\.h$")

# ============================================================================ #
# Examples
# ============================================================================ #
add_subdirectory (example)
