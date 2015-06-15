include( ExternalProject )

set( base "${CMAKE_BINARY_DIR}" )
set_property( DIRECTORY PROPERTY EP_BASE ${base} )

set( shared ON ) # use for BUILD_SHARED_LIBS on all subsequent projects
set( testing OFF ) # use for BUILD_TESTING on all subsequent projects
set( build_type "Debug" )
if( CMAKE_BUILD_TYPE )
  set( build_type "${CMAKE_BUILD_TYPE}" )
endif()

set( Covalic_DEPENDS "" )

set( gen "${CMAKE_GENERATOR}" )

if( NOT GIT_EXECUTABLE )
  find_package( Git REQUIRED )
endif( NOT GIT_EXECUTABLE )

option( GIT_PROTOCOL_HTTP
  "Use HTTP for git access (useful if behind a firewall)" OFF )
if( GIT_PROTOCOL_HTTP )
  set( GIT_PROTOCOL "http" CACHE STRING "Git protocol for file transfer" )
else( GIT_PROTOCOL_HTTP )
  set( GIT_PROTOCOL "git" CACHE STRING "Git protocol for file transfer" )
endif( GIT_PROTOCOL_HTTP )
mark_as_advanced( GIT_PROTOCOL )

set( proj VTK )
ExternalProject_Add( ${proj}
  #GIT_REPOSITORY "${GIT_PROTOCOL}://vtk.org/VTK.git"
  GIT_REPOSITORY "https://gitlab.kitware.com/vtk/vtk.git"
  GIT_TAG "v6.1.0"
  SOURCE_DIR "${CMAKE_BINARY_DIR}/VTK"
  BINARY_DIR VTK-Build
  CMAKE_GENERATOR ${gen}
  CMAKE_ARGS
  -Dgit_EXECUTABLE:FILEPATH=${GIT_EXECUTABLE}
  -DCMAKE_CXX_FLAGS:STRING=${CMAKE_CXX_FLAGS}
  -DCMAKE_C_FLAGS:STRING=${CMAKE_C_FLAGS}
  -DCMAKE_EXE_LINKER_FLAGS:STRING=${CMAKE_EXE_LINKER_FLAGS}
  -DCMAKE_SHARED_LINKER_FLAGS:STRING=${CMAKE_SHARED_LINKER_FLAGS}
  -DCMAKE_BUILD_TYPE:STRING=${build_type}
  -DBUILD_SHARED_LIBS:BOOL=${shared}
  -DBUILD_EXAMPLES:BOOL=OFF
  -DBUILD_TESTING:BOOL=OFF
  INSTALL_COMMAND ""
)

set( VTK_DIR "${base}/VTK-Build" )

##
## Check if sytem ITK or superbuild ITK (or ITKv4)
##
if( NOT USE_SYSTEM_ITK )
  ##
  ## Insight
  ##
  if( Covalic_USE_ITKV4 )
    set( proj Insight )
    ExternalProject_Add( ${proj}
      GIT_REPOSITORY "${GIT_PROTOCOL}://itk.org/ITK.git"
      GIT_TAG "origin/master"
      SOURCE_DIR "${CMAKE_BINARY_DIR}/Insight"
      BINARY_DIR Insight-Build
      CMAKE_GENERATOR ${gen}
      CMAKE_ARGS
        -Dgit_EXECUTABLE:FILEPATH=${GIT_EXECUTABLE}
        -DCMAKE_CXX_FLAGS:STRING=${CMAKE_CXX_FLAGS}
        -DCMAKE_C_FLAGS:STRING=${CMAKE_C_FLAGS}
        -DCMAKE_EXE_LINKER_FLAGS:STRING=${CMAKE_EXE_LINKER_FLAGS}
        -DCMAKE_SHARED_LINKER_FLAGS:STRING=${CMAKE_SHARED_LINKER_FLAGS}
        -DCMAKE_BUILD_TYPE:STRING=${build_type}
        -DBUILD_SHARED_LIBS:BOOL=${shared}
        -DBUILD_EXAMPLES:BOOL=OFF
        -DBUILD_TESTING:BOOL=OFF
        -DITK_USE_REVIEW:BOOL=ON
        -DITK_USE_OPTIMIZED_REGISTRATION_METHODS:BOOL=ON
      INSTALL_COMMAND ""
      )

  else( Covalic_USE_ITKV4 )
    set( proj Insight )
    ExternalProject_Add( ${proj}
      GIT_REPOSITORY "${GIT_PROTOCOL}://github.com/Slicer/ITK.git"
      GIT_TAG "origin/slicer-4.0"
      SOURCE_DIR "${CMAKE_BINARY_DIR}/Insight"
      BINARY_DIR Insight-Build
      CMAKE_GENERATOR ${gen}
      CMAKE_ARGS
        -Dgit_EXECUTABLE:FILEPATH=${GIT_EXECUTABLE}
        -DCMAKE_CXX_FLAGS:STRING=${CMAKE_CXX_FLAGS}
        -DCMAKE_C_FLAGS:STRING=${CMAKE_C_FLAGS}
        -DCMAKE_EXE_LINKER_FLAGS:STRING=${CMAKE_EXE_LINKER_FLAGS}
        -DCMAKE_SHARED_LINKER_FLAGS:STRING=${CMAKE_SHARED_LINKER_FLAGS}
        -DCMAKE_BUILD_TYPE:STRING=${build_type}
        -DBUILD_SHARED_LIBS:BOOL=${shared}
        -DBUILD_EXAMPLES:BOOL=OFF
        -DBUILD_TESTING:BOOL=OFF
        -DITK_USE_REVIEW:BOOL=ON
        -DITK_USE_OPTIMIZED_REGISTRATION_METHODS:BOOL=ON
      INSTALL_COMMAND ""
      )
  endif( Covalic_USE_ITKV4 )
  set( ITK_DIR "${base}/Insight-Build" )

  set( Covalic_DEPENDS ${Covalic_DEPENDS} "Insight" )

endif( NOT USE_SYSTEM_ITK )

##
## Slicer execution model
##

set( proj SlicerExecutionModel )

if( NOT USE_SYSTEM_ITK )
  # Depends on ITK if ITK was build using superbuild
  set( SlicerExecutionModel_DEPENDS "Insight" )
else( NOT USE_SYSTEM_ITK )
  set( SlicerExecutionModel_DEPENDS "" )
endif( NOT USE_SYSTEM_ITK )


ExternalProject_Add( ${proj}
  GIT_REPOSITORY "https://github.com/Slicer/SlicerExecutionModel"
  SOURCE_DIR "${CMAKE_BINARY_DIR}/SlicerExecutionModel"
  BINARY_DIR SlicerExecutionModel-Build
  CMAKE_GENERATOR ${gen}
  CMAKE_ARGS
    -DCMAKE_CXX_FLAGS:STRING=${CMAKE_CXX_FLAGS}
    -DCMAKE_C_FLAGS:STRING=${CMAKE_C_FLAGS}
    -DCMAKE_EXE_LINKER_FLAGS:STRING=${CMAKE_EXE_LINKER_FLAGS}
    -DCMAKE_SHARED_LINKER_FLAGS:STRING=${CMAKE_SHARED_LINKER_FLAGS}
    -DCMAKE_BUILD_TYPE:STRING=${build_type}
    -DBUILD_SHARED_LIBS:BOOL=${shared}
    -DBUILD_EXAMPLES:BOOL=OFF
    -DBUILD_TESTING:BOOL=OFF
    -DITK_DIR:PATH=${ITK_DIR}
  INSTALL_COMMAND ""
  DEPENDS ${SlicerExecutionModel_DEPENDS}
  )

set( TCLAP_DIR "${base}/SlicerExecutionModel-Build/tclap" )
set( ModuleDescriptionParser_DIR "${base}/SlicerExecutionModel-Build/ModuleDescriptionParser" )
set( GenerateCLP_DIR "${base}/SlicerExecutionModel-Build/GenerateCLP" )

set( Covalic_DEPENDS ${Covalic_DEPENDS} "SlicerExecutionModel" "VTK" )

##
## Covalic - Normal Build
##
if( NOT Covalic_USE_ITKV4 )
set( proj Covalic )
ExternalProject_Add( ${proj}
  DOWNLOAD_COMMAND ""
  SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}"
  BINARY_DIR Covalic-Build
  CMAKE_GENERATOR ${gen}
  CMAKE_ARGS
    -DBUILDNAME:STRING=${BUILDNAME}
    -DSITE:STRING=${SITE}
    -DMAKECOMMAND:STRING=${MAKECOMMAND}
    -DCMAKE_BUILD_TYPE:STRING=${build_type}
    -DBUILD_SHARED_LIBS:BOOL=${BUILD_SHARED_LIBS}
    -DBUILD_EXAMPLES:BOOL=${BUILD_EXAMPLES}
    -DBUILD_TESTING:BOOL=${BUILD_TESTING}
    -DBUILD_DOCUMENTATION:BOOL=${BUILD_DOCUMENTATION}
    -DCMAKE_SHARED_LINKER_FLAGS:STRING=${CMAKE_SHARED_LINKER_FLAGS}
    -DCMAKE_EXE_LINKER_FLAGS:STRING=${CMAKE_EXE_LINKER_FLAGS}
    -DCMAKE_CXX_FLAGS:STRING=${CMAKE_CXX_FLAGS}
    -DCMAKE_C_FLAGS:STRING=${CMAKE_C_FLAGS}
    -DCovalic_USE_SUPERBUILD:BOOL=FALSE
    -DCovalic_USE_ITKV4:BOOL=${Covalic_USE_ITKV4}
    -DCovalic_EXECUTABLE_DIRS:BOOL=${Covalic_EXECUTABLE_DIRS}
    -DITK_DIR:PATH=${ITK_DIR}
    -DVTK_DIR:PATH=${VTK_DIR}
    -DGenerateCLP_DIR:PATH=${GenerateCLP_DIR}
  INSTALL_COMMAND ""
  DEPENDS
    ${Covalic_DEPENDS}
 )

##
## Covalic - ITKv4 Build
##
else( NOT Covalic_USE_ITKV4 )
set( proj Covalic )
ExternalProject_Add( ${proj}
  DOWNLOAD_COMMAND ""
  SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}"
  BINARY_DIR Covalic-Build
  CMAKE_GENERATOR ${gen}
  CMAKE_ARGS
    -DBUILDNAME:STRING=${BUILDNAME}
    -DSITE:STRING=${SITE}
    -DMAKECOMMAND:STRING=${MAKECOMMAND}
    -DCMAKE_BUILD_TYPE:STRING=${build_type}
    -DBUILD_SHARED_LIBS:BOOL=${BUILD_SHARED_LIBS}
    -DBUILD_EXAMPLES:BOOL=${BUILD_EXAMPLES}
    -DBUILD_TESTING:BOOL=${BUILD_TESTING}
    -DBUILD_DOCUMENTATION:BOOL=${BUILD_DOCUMENTATION}
    -DCMAKE_SHARED_LINKER_FLAGS:STRING=${CMAKE_SHARED_LINKER_FLAGS}
    -DCMAKE_EXE_LINKER_FLAGS:STRING=${CMAKE_EXE_LINKER_FLAGS}
    -DCMAKE_CXX_FLAGS:STRING=${CMAKE_CXX_FLAGS}
    -DCMAKE_C_FLAGS:STRING=${CMAKE_C_FLAGS}
    -DCovalic_USE_SUPERBUILD:BOOL=FALSE
    -DCovalic_USE_ITKV4:BOOL=${Covalic_USE_ITKV4}
    -DCovalic_EXECUTABLE_DIRS:BOOL=${Covalic_EXECUTABLE_DIRS}
    -DITK_DIR:PATH=${ITK_DIR}
    -DVTK_DIR:PATH=${VTK_DIR}
    -DGenerateCLP_DIR:PATH=${GenerateCLP_DIR}
  INSTALL_COMMAND ""
  DEPENDS
    ${Covalic_DEPENDS}
 )
endif( NOT Covalic_USE_ITKV4 )
