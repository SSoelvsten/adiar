set_property(GLOBAL PROPERTY USE_FOLDERS On)

# ============================================================================ #
# Link up files for the library

set(HEADERS
  # adiar
  access_mode.h
  adiar.h
  assignment.h
  bool_op.h
  builder.h
  data.h
  deprecated.h
  domain.h
  file.h
  map.h
  memory_mode.h
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
  internal/block_size.h
  internal/cnl.h
  internal/cut.h
  internal/dd.h
  internal/dd_func.h
  internal/dot.h
  internal/memory.h
  internal/util.h

  # internal/algorithms
  internal/algorithms/build.h
  internal/algorithms/convert.h
  internal/algorithms/count.h
  internal/algorithms/intercut.h
  internal/algorithms/nested_sweeping.h
  internal/algorithms/pred.h
  internal/algorithms/prod2.h
  internal/algorithms/quantify.h
  internal/algorithms/reduce.h
  internal/algorithms/substitution.h
  internal/algorithms/traverse.h

  # adiar/internal/data_structures
  internal/data_structures/level_merger.h
  internal/data_structures/levelized_priority_queue.h
  internal/data_structures/priority_queue.h
  internal/data_structures/sorter.h

  # adiar/internal/data_types
  internal/data_types/arc.h
  internal/data_types/convert.h
  internal/data_types/level_info.h
  internal/data_types/node.h
  internal/data_types/ptr.h
  internal/data_types/request.h
  internal/data_types/tuple.h
  internal/data_types/uid.h

  # adiar/internal/io
  internal/io/file.h
  internal/io/file_stream.h
  internal/io/file_writer.h

  internal/io/levelized_file.h
  internal/io/levelized_file_stream.h
  internal/io/levelized_file_writer.h

  internal/io/shared_file_ptr.h

  internal/io/arc_file.h
  internal/io/arc_stream.h
  internal/io/arc_writer.h

  internal/io/node_file.h
  internal/io/node_random_access.h
  internal/io/node_stream.h
  internal/io/node_writer.h
)

set(SOURCES
  # adiar/
  access_mode.cpp
  adiar.cpp
  domain.cpp
  statistics.cpp
  memory_mode.cpp

  # adiar/bdd/
  bdd/apply.cpp
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

  # adiar/internal/algorithms
  internal/algorithms/count.cpp
  internal/algorithms/intercut.cpp
  internal/algorithms/pred.cpp
  internal/algorithms/prod2.cpp
  internal/algorithms/quantify.cpp
  internal/algorithms/reduce.cpp
  internal/algorithms/substitution.cpp

  # adiar/internal/data_structures
  internal/data_structures/levelized_priority_queue.cpp

  # adiar/internal/io
  internal/io/arc_file.cpp
  internal/io/node_file.cpp
)

# ============================================================================ #
# Settings
if (ADIAR_SHARED)
  # TODO: GitHub Issue #200
else()
  add_library(${PROJECT_NAME} STATIC ${HEADERS} ${SOURCES})
endif()

if (ADIAR_STATS)
  target_compile_definitions(${PROJECT_NAME} PUBLIC ADIAR_STATS)
endif()

# ============================================================================ #
# Link dependencies
target_link_libraries(${PROJECT_NAME} PUBLIC tpie)
target_link_libraries(${PROJECT_NAME} PUBLIC Cnl)

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

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)
