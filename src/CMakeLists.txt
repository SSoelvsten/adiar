set_property(GLOBAL PROPERTY USE_FOLDERS On)

# ============================================================================ #
# Link up files for the library
set(HEADERS
  data.h
)

set(SOURCES
  data.cpp
)

if (COOM_SHARED)
  add_library(coom SHARED ${HEADERS} ${SOURCES})
else()
  add_library(coom STATIC ${HEADERS} ${SOURCES})
endif()

include(GenerateExportHeader)
generate_export_header(coom DEFINE_NO_DEPRECATED)

# ============================================================================ #
# Link dependencies
# target_link_libraries(coom tpie)

# ============================================================================ #
# Setup as library

target_include_directories(coom PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
set_target_properties(coom PROPERTIES
  # Language settings
  LINKER_LANGUAGE CXX
  C_STANDARD 17
  C_STANDARD_REQUIRED ON
  CXX_STANDARD 17
  CXX_STANDARD_REQUIRED ON
)

install(TARGETS coom EXPORT coom DESTINATION bib)
