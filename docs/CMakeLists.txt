# Path to output doxygen configuration file
SET(DOXYFILE ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

find_package(Doxygen)

if (DOXYGEN)
  # exclude internal documentation
  set(USE_INTERNAL_DOCS "NO")

  # configure doxygen files
  set(DOXYGEN_IN ${PROJECT_SOURCE_DIR}/docs/Doxyfile.in)
  set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/docs/Doxyfile)

  configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)

  # add target
  add_custom_target( adiar_docs ALL
    COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Generating API documentation with Doxygen"
    VERBATIM )
else (DOXYGEN)
  message(STATUS "Doxygen not found, API documentation cannot be generated.")
endif(DOXYGEN)

# TODO copy over assets
