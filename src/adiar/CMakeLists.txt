set_property(GLOBAL PROPERTY USE_FOLDERS On)

# ============================================================================ #
# Link up files for the library

set(HEADERS
  # core files
  adiar.h
  builder.h
  data.h
  domain.h
  file.h
  file_stream.h
  file_writer.h

  # deprecated
  deprecated.h

  # bdd files
  bdd.h
  bdd/bdd.h
  bdd/bdd_policy.h
  bdd/if_then_else.h

  # zdd files
  zdd.h
  zdd/build.h
  zdd/zdd.h
  zdd/zdd_policy.h

  # internal
  internal/assert.h
  internal/build.h
  internal/cnl.h
  internal/convert.h
  internal/count.h
  internal/cut.h
  internal/decision_diagram.h
  internal/dot.h
  internal/intercut.h
  internal/memory.h
  internal/pred.h
  internal/levelized_priority_queue.h
  internal/priority_queue.h
  internal/product_construction.h
  internal/quantify.h
  internal/reduce.h
  internal/sorter.h
  internal/substitution.h
  internal/traverse.h
  internal/tuple.h
  internal/util.h

  # statistics
  statistics.h
)

set(SOURCES
  # core files
  adiar.cpp
  domain.cpp
  file.cpp

  # bdd files
  bdd/apply.cpp
  bdd/assignment.cpp
  bdd/bdd.cpp
  bdd/build.cpp
  bdd/count.cpp
  bdd/evaluate.cpp
  bdd/if_then_else.cpp
  bdd/negate.cpp
  bdd/pred.cpp
  bdd/quantify.cpp
  bdd/restrict.cpp

  # zdd files
  zdd/binop.cpp
  zdd/build.cpp
  zdd/change.cpp
  zdd/complement.cpp
  zdd/contains.cpp
  zdd/count.cpp
  zdd/elem.cpp
  zdd/expand.cpp
  zdd/pred.cpp
  zdd/project.cpp
  zdd/subset.cpp
  zdd/zdd.cpp

  # internal
  internal/count.cpp
  internal/intercut.cpp
  internal/memory.cpp
  internal/pred.cpp
  internal/product_construction.cpp
  internal/levelized_priority_queue.cpp
  internal/quantify.cpp
  internal/reduce.cpp
  internal/sorter.cpp
  internal/substitution.cpp

  # statistics
  statistics.cpp
)

# ============================================================================ #
# Settings
if (ADIAR_SHARED)
  # TODO: GitHub Issue #200
else()
  add_library(${PROJECT_NAME} STATIC ${HEADERS} ${SOURCES})
endif()

if (ADIAR_STATS_EXTRA OR ADIAR_STATS)
  target_compile_definitions(${PROJECT_NAME} PRIVATE ADIAR_STATS)
endif()

# ============================================================================ #
# Link dependencies
target_link_libraries(${PROJECT_NAME} tpie)
target_link_libraries(${PROJECT_NAME} Cnl)

# ============================================================================ #
# Setup as library

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/..
                                                  ${CMAKE_CURRENT_SOURCE_DIR}/..)

set_target_properties(${PROJECT_NAME} PROPERTIES
  # Language settings
  LINKER_LANGUAGE CXX
  C_STANDARD 17
  C_STANDARD_REQUIRED ON
  CXX_STANDARD 17
  CXX_STANDARD_REQUIRED ON
  CXX_VISIBILITY_PRESET hidden

  # Visual Studio
  FOLDER ${PROJECT_NAME}

  # Project Information
  VERSION ${PROJECT_VERSION}
)

target_compile_features(adiar PUBLIC cxx_std_17)
