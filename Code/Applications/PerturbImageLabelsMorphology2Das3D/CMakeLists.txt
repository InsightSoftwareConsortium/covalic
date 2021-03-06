# If you follow the SampleCLIApplication format, you only need to change the
# following line to configure this CMakeLists.txt file.
project( PerturbImageLabelsMorphology2Das3D )

cmake_minimum_required( VERSION 2.8 )
if( COMMAND CMAKE_POLICY )
  cmake_policy( SET CMP0003 NEW )
endif( COMMAND CMAKE_POLICY )

# Disable MSVC 8 warnings
if( WIN32 )
  option( DISABLE_MSVC8_DEPRECATED_WARNINGS "Disable Visual Studio 8 deprecated warnings" ON )
  mark_as_advanced( FORCE DISABLE_MSVC8_DEPRECATED_WARNINGS )
  if( DISABLE_MSVC8_DEPRECATED_WARNINGS )
    add_definitions( -D_CRT_SECURE_NO_DEPRECATE )
  endif( DISABLE_MSVC8_DEPRECATED_WARNINGS )
endif( WIN32)

# Find ITK
find_package( ITK REQUIRED )
include( ${USE_ITK_FILE} )

# Find GenerateCLP
find_package( GenerateCLP REQUIRED )
include( ${GenerateCLP_USE_FILE} )

# Include Utilities to access covalicCLIHelperFunctions.h
include_directories(
  ${Covalic_SOURCE_DIR}/Utilities
  ${CMAKE_CURRENT_SOURCE_DIR}/../Common
  )

set( PROJECT_SOURCE
  ${PROJECT_NAME}.cxx
  )

generateclp( PROJECT_SOURCE ${PROJECT_NAME}.xml )

# Build the shared library
add_library( ${PROJECT_NAME}Module SHARED ${PROJECT_SOURCE} )
set_target_properties( ${PROJECT_NAME}Module
                       PROPERTIES COMPILE_FLAGS "-Dmain=ModuleEntryPoint" )
target_link_libraries( ${PROJECT_NAME}Module ${ITK_LIBRARIES} ${VTK_LIBRARIES} )

add_executable( ${PROJECT_NAME}
                ${Covalic_SOURCE_DIR}/Utilities/covalicCLISharedLibraryWrapper.cxx )
target_link_libraries( ${PROJECT_NAME} ${PROJECT_NAME}Module )

slicer3_set_plugins_output_path( ${PROJECT_NAME}Module )
slicer3_set_plugins_output_path( ${PROJECT_NAME} )
set( TARGETS
     ${PROJECT_NAME}Module
     ${PROJECT_NAME} )
slicer3_install_plugins( ${TARGETS} )


# copy over the Midas-Integration files to the built project
file(GLOB MIDAS_INTEGRATION_FILES ./Midas-Integration *)
file(COPY ${MIDAS_INTEGRATION_FILES} DESTINATION .)
