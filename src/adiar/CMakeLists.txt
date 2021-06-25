set_property(GLOBAL PROPERTY USE_FOLDERS On)

# ============================================================================ #
# Link up files for the library

set(HEADERS
  # core files
  assert.h
  adiar.h
  data.h
  file.h
  file_stream.h
  file_writer.h
  tuple.h

  # bdd files
  bdd/apply.h
  bdd/assignment.h
  bdd/bdd.h
  bdd/build.h
  bdd/count.h
  bdd/evaluate.h
  bdd/if_then_else.h
  bdd/negate.h
  bdd/quantify.h
  bdd/restrict.h

  # zdd files
  zdd/build.h
  zdd/count.h
  zdd/pred.h
  zdd/set_op.h
  zdd/zdd.h

  # debug files
  dot.h

  # internal
  internal/build.h
  internal/count.h
  internal/decision_diagram.h
  internal/pred.h
  internal/levelized_priority_queue.h
  internal/product_construction.h
  internal/reduce.h
)

set(SOURCES
  # core files
  adiar.cpp
  data.cpp
  file.cpp
  file_stream.cpp
  file_writer.cpp

  # bdd files
  bdd/apply.cpp
  bdd/assignment.cpp
  bdd/bdd.cpp
  bdd/build.cpp
  bdd/count.cpp
  bdd/evaluate.cpp
  bdd/if_then_else.cpp
  bdd/negate.cpp
  bdd/quantify.cpp
  bdd/restrict.cpp

  # zdd files
  zdd/build.cpp
  zdd/count.cpp
  zdd/pred.cpp
  zdd/set_op.cpp
  zdd/zdd.cpp

  # debug files
  dot.cpp

  # internal
  internal/decision_diagram.cpp
  internal/pred.cpp
  internal/levelized_priority_queue.cpp
  internal/reduce.cpp
)

if (ADIAR_SHARED)
  add_library(adiar SHARED ${HEADERS} ${SOURCES})
else()
  add_library(adiar STATIC ${HEADERS} ${SOURCES})
endif()

include(GenerateExportHeader)
generate_export_header(adiar
  EXPORT_MACRO_NAME ADIAR_API
)

# ============================================================================ #
# Link dependencies
target_link_libraries(adiar tpie)

# ============================================================================ #
# Setup as library

target_include_directories(adiar PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/..
                                        ${CMAKE_CURRENT_SOURCE_DIR}/..)

set_target_properties(adiar PROPERTIES
  # Language settings
  LINKER_LANGUAGE CXX
  C_STANDARD 17
  C_STANDARD_REQUIRED ON
  CXX_STANDARD 17
  CXX_STANDARD_REQUIRED ON
  CXX_VISIBILITY_PRESET hidden

  # Linkage?
  FOLDER adiar

  # Other
  VERSION ${PROJECT_VERSION}
  POSITION_INDEPENDENT_CODE ON
)

target_compile_features(adiar PUBLIC cxx_std_17)

install(TARGETS adiar
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        INCLUDES DESTINATION ${LIBLEGACY_INCLUDE_DIRS}
)
