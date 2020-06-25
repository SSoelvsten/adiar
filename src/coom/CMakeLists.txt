set_property(GLOBAL PROPERTY USE_FOLDERS On)

# ============================================================================ #
# Link up files for the library

set(HEADERS
  apply.h
  coom.h
  data.h
  data_pty.h
  evaluate.h
  reduce.h
  restrict.h
)

set(SOURCES
  apply.cpp
  data.cpp
  data_pty.cpp
  evaluate.cpp
  reduce.cpp
  restrict.cpp
)

if (COOM_SHARED)
  add_library(coom SHARED ${HEADERS} ${SOURCES})
else()
  add_library(coom STATIC ${HEADERS} ${SOURCES})
endif()

if (COOM_DEBUG)
  target_compile_definitions(coom PRIVATE COOM_DEBUG=1)
endif()

if (COOM_ASSERT)
    target_compile_definitions(coom PRIVATE COOM_ASSERT=1)
endif()

include(GenerateExportHeader)
generate_export_header(coom)

# ============================================================================ #
# Link dependencies
target_link_libraries(coom PRIVATE tpie)

# ============================================================================ #
# Setup as library

target_include_directories(coom PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/..
                                       ${CMAKE_CURRENT_SOURCE_DIR}/..)

set_target_properties(coom PROPERTIES
  # Language settings
  LINKER_LANGUAGE CXX
  C_STANDARD 17
  C_STANDARD_REQUIRED ON
  CXX_STANDARD 17
  CXX_STANDARD_REQUIRED ON
  CXX_VISIBILITY_PRESET hidden

  # Linkage?
  FOLDER coom

  # Other
  VERSION ${PROJECT_VERSION}
  POSITION_INDEPENDENT_CODE ON
)

install(TARGETS coom
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)


# ============================================================================ #
# TODO: DELETE WHEN PRINT OUTPUT IS NOT NEEDED
add_executable (coom_main main.cpp)
target_link_libraries(coom_main tpie)
target_link_libraries(coom_main coom)

target_compile_definitions(coom_main PRIVATE COOM_DEBUG=1 COOM_ASSERT=1)
