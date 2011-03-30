#  We use Slicer3 variable names to simplify porting to Slicer3
set( Slicer3_INSTALL_BIN_DIR "bin" )
set( Slicer3_INSTALL_LIB_DIR "lib/covalic" )
set( Slicer3_INSTALL_INCLUDE_DIR "include/covalic" )
set( Slicer3_INSTALL_SHARE_DIR "share/covalic" )
set( Slicer3_INSTALL_ITKFACTORIES_DIR "${Slicer3_INSTALL_LIB_DIR}/ITKFactories" )

set( Slicer3_INSTALL_PLUGINS_BIN_DIR "${Slicer3_INSTALL_LIB_DIR}/Plugins" )
set( Slicer3_INSTALL_PLUGINS_LIB_DIR "${Slicer3_INSTALL_LIB_DIR}/Plugins" )
set( Slicer3_INSTALL_PLUGINS_INCLUDE_DIR
  "${Slicer3_INSTALL_INCLUDE_DIR}/Plugins" )
set( Slicer3_INSTALL_PLUGINS_SHARE_DIR "${Slicer3_INSTALL_SHARE_DIR}/Plugins" )
set( Slicer3_INSTALL_PLUGINS_CACHE_DIR "${Slicer3_INSTALL_LIB_DIR}/PluginsCache" )

#-----------------------------------------------------------------------------
# Set the default output paths for one or more plugins/CLP
# 
macro(slicer3_set_plugins_output_path)
  set_target_properties(${ARGN}
    PROPERTIES 
    RUNTIME_OUTPUT_DIRECTORY 
    "${CMAKE_BINARY_DIR}/${Slicer3_INSTALL_PLUGINS_BIN_DIR}"
    LIBRARY_OUTPUT_DIRECTORY 
    "${CMAKE_BINARY_DIR}/${Slicer3_INSTALL_PLUGINS_LIB_DIR}"
    ARCHIVE_OUTPUT_DIRECTORY 
    "${CMAKE_BINARY_DIR}/${Slicer3_INSTALL_PLUGINS_LIB_DIR}"
    )
endmacro(slicer3_set_plugins_output_path)

#-----------------------------------------------------------------------------
# Install one or more plugins to the default plugin location
# 
macro(slicer3_install_plugins)
  install(TARGETS ${ARGN}
    RUNTIME DESTINATION ${Slicer3_INSTALL_PLUGINS_BIN_DIR} COMPONENT RuntimeLibraries
    LIBRARY DESTINATION ${Slicer3_INSTALL_PLUGINS_LIB_DIR} COMPONENT RuntimeLibraries
    ARCHIVE DESTINATION ${Slicer3_INSTALL_PLUGINS_LIB_DIR} COMPONENT RuntimeLibraries
    )
endmacro(slicer3_install_plugins)

