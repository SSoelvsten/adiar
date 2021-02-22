# Parts of this CMakeLists.txt is derived from the files of the tpie and
# sylvan projects.

cmake_minimum_required(VERSION 3.8)
project (adiar
  VERSION 1.0
  DESCRIPTION "Adiar, an external memory decision diagram library"
  LANGUAGES CXX
)

message(STATUS "CMake build configuration for Adiar ${PROJECT_VERSION}")
enable_language(CXX)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(CODE_COVERAGE "Build with coverage profiling" OFF)
option(ADIAR_TEST "Build adiar unit tests" OFF)

if (CODE_COVERAGE)
  set(CMAKE_C_FLAGS "-g -O0 -Wall -W -fprofile-arcs -ftest-coverage ${CMAKE_C_FLAGS}")
  set(CMAKE_CXX_FLAGS "-g -O0 -Wall -fprofile-arcs -ftest-coverage ${CMAKE_CXX_FLAGS}")
  set(CMAKE_EXE_LINKER_FLAGS "-fprofile-arcs -ftest-coverage")
elseif (ADIAR_TEST)
  set(CMAKE_C_FLAGS "-g -O2 ${CMAKE_C_FLAGS}")
  set(CMAKE_CXX_FLAGS "-g -O2 ${CMAKE_CXX_FLAGS}")
endif()

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
if (ADIAR_TEST OR CODE_COVERAGE)
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
