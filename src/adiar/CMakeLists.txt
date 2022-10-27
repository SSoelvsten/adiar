set_property(GLOBAL PROPERTY USE_FOLDERS On)

# ============================================================================ #
# Link up files for the library

set(HEADERS
  # adiar
  adiar.h
  assignment.h
  bool_op.h
  builder.h
  data.h
  deprecated.h
  domain.h
  file.h
  file_stream.h
  file_writer.h
  label.h
  statistics.h

  # adiar/bdd
  bdd.h
  bdd/bdd.h
  bdd/bdd_policy.h
  bdd/if_then_else.h

  # adiar/zdd
  zdd.h
  zdd/build.h
  zdd/zdd.h
  zdd/zdd_policy.h

  # adiar/internal
  internal/assert.h
  internal/cnl.h
  internal/cut.h
  internal/decision_diagram.h
  internal/dot.h
  internal/memory.h
  internal/util.h

  # internal/algorithms
  internal/algorithms/build.h
  internal/algorithms/convert.h
  internal/algorithms/count.h
  internal/algorithms/intercut.h
  internal/algorithms/pred.h
  internal/algorithms/product_construction.h
  internal/algorithms/quantify.h
  internal/algorithms/reduce.h
  internal/algorithms/substitution.h
  internal/algorithms/traverse.h

  # adiar/internal/data_structures
  internal/data_structures/levelized_priority_queue.h
  internal/data_structures/priority_queue.h
  internal/data_structures/sorter.h

  # adiar/internal/data_types
  internal/data_types/arc.h
  internal/data_types/convert.h
  internal/data_types/id.h
  internal/data_types/level_info.h
  internal/data_types/node.h
  internal/data_types/ptr.h
  internal/data_types/tuple.h
  internal/data_types/triple.h
  internal/data_types/uid.h
)

set(SOURCES
  # adiar/
  adiar.cpp
  domain.cpp
  file.cpp
  statistics.cpp

  # adiar/bdd/
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

  # adiar/zdd/
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

  # adiar/internal
  internal/memory.cpp

  # adiar/internal/algorithms
  internal/algorithms/count.cpp
  internal/algorithms/intercut.cpp
  internal/algorithms/pred.cpp
  internal/algorithms/product_construction.cpp
  internal/algorithms/quantify.cpp
  internal/algorithms/reduce.cpp
  internal/algorithms/substitution.cpp

  # adiar/internal/data_structures
  internal/data_structures/levelized_priority_queue.cpp
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
  CXX_VISIBILITY_PRESET default

  # Visual Studio
  FOLDER ${PROJECT_NAME}

  # Project Information
  VERSION ${PROJECT_VERSION}
)

target_compile_features(adiar PUBLIC cxx_std_17)
