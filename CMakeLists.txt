project(Covalic)

#-----------------------------------------------------------------------------
# Require CMake 2.8 for the superbuild functionality
cmake_minimum_required(VERSION 2.8)

option( BUILD_DOCUMENTATION "Generate Documentation for COVALIC" NO )

#-----------------------------------------------------------------------------
# ITK Setup
option( USE_SYSTEM_ITK
  "Exclude ITK from SuperBuild and use an existing build instead." OFF )
if( USE_SYSTEM_ITK )
  find_package( ITK REQUIRED )
  include( ${ITK_USE_FILE} )
endif( USE_SYSTEM_ITK )

#-----------------------------------------------------------------------------
# ITKv4 Switch
option( Covalic_USE_ITKV4
  "Compile with ITKv4. Only supported applications will be built."
  OFF )
mark_as_advanced( Covalic_USE_ITKV4 )

#-----------------------------------------------------------------------------
# Superbuild setup
option( Covalic_USE_SUPERBUILD
  "Build Covalic and the projects it depends on via SuperBuild.cmake." ON )

if( Covalic_USE_SUPERBUILD )

  include( "${CMAKE_CURRENT_SOURCE_DIR}/CMake/SuperBuild.cmake" )
  return()

else( Covalic_USE_SUPERBUILD )

  include( ${CMAKE_CURRENT_SOURCE_DIR}/CMake/Slicer3PluginsMacros.cmake )

  find_package( ITK REQUIRED )
  include( ${ITK_USE_FILE} )

  find_package( GenerateCLP REQUIRED )
  include( ${GenerateCLP_USE_FILE} )

endif( Covalic_USE_SUPERBUILD )

add_subdirectory( Documentation )
add_subdirectory( Code )
add_subdirectory( Utilities )