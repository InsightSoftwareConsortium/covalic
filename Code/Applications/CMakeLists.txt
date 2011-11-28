cmake_minimum_required(VERSION 2.6)

project(validation_apps)

include (${CMAKE_ROOT}/Modules/FindITK.cmake)
if (USE_ITK_FILE)
  include(${USE_ITK_FILE})
endif (USE_ITK_FILE)

find_package(VTK)
if (VTK_FOUND)
  include(${USE_VTK_FILE})
else (VTK_FOUND)
  message(FATAL_ERROR "VTK not found. Please set VTK_DIR.")
endif (VTK_FOUND)

set(VTK_LIBRARIES
  vtkFiltering
  vtkGraphics
  vtkHybrid
  vtkImaging
  vtkRendering
  vtkIO
  vtkCommon
)


include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/../Converters
  ${CMAKE_CURRENT_SOURCE_DIR}/../Metrics
)

# Metrics for binary images
add_subdirectory(ValidateImageDice)
add_subdirectory(ValidateImageJaccard)
add_subdirectory(ValidateImageAveDist)

# Metrics for label images
add_subdirectory(ValidateImageKappa)

# Metrics for surfaces
add_subdirectory(ValidateSurfaceCurrents)
add_subdirectory(ValidateSurfaceHaussdorff)
