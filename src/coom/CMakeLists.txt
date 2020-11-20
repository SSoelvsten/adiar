set_property(GLOBAL PROPERTY USE_FOLDERS On)

# ============================================================================ #
# Link up files for the library

set(HEADERS
  # core files
  assert.h
  coom.h
  data.h
  file.h
  file_stream.h
  file_writer.h
  homomorphism.h
  priority_queue.h
  reduce.h
  tuple.h
  union.h
  util.h

  # bdd files
  bdd/apply.h
  bdd/assignment.h
  bdd/bdd.h
  bdd/build.h
  bdd/count.h
  bdd/evaluate.h
  bdd/negate.h
  bdd/quantify.h
  bdd/restrict.h

  # debug files
  dot.h
)

set(SOURCES
  # core files
  assert.cpp
  coom.cpp
  data.cpp
  file.cpp
  file_stream.cpp
  file_writer.cpp
  homomorphism.cpp
  priority_queue.cpp
  reduce.cpp

  # bdd files
  bdd/apply.cpp
  bdd/bdd.cpp
  bdd/build.cpp
  bdd/count.cpp
  bdd/evaluate.cpp
  bdd/negate.cpp
  bdd/quantify.cpp
  bdd/restrict.cpp

  # debug files
  dot.cpp
)

if (COOM_SHARED)
  add_library(coom SHARED ${HEADERS} ${SOURCES})
else()
  add_library(coom STATIC ${HEADERS} ${SOURCES})
endif()

#if (COOM_ASSERT)
  target_compile_definitions(coom PRIVATE COOM_ASSERT=1)
#endif()

include(GenerateExportHeader)
generate_export_header(coom)

# ============================================================================ #
# Link dependencies
target_link_libraries(coom tpie)

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
