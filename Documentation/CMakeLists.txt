if( BUILD_DOCUMENTATION )
  find_package( Doxygen )
  add_custom_target( 
    documentation
    COMMAND ${DOXYGEN_DOT_EXECUTABLE} -Tpng -o${CMAKE_CURRENT_BINARY_DIR}/classDiagram.png ${CMAKE_CURRENT_SOURCE_DIR}/classDiagram.dot )
endif()