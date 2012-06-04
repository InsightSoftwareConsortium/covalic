cmake_minimum_required(VERSION 2.6)

project(validation_tests)

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
  ${CMAKE_CURRENT_SOURCE_DIR}/../Applications/Common
)

add_executable(testImageMetrics testImageMetrics.cxx)
add_executable(testSurfMetrics
  testSurfMetrics.cxx
  ../Metrics/SurfaceToSurfaceMetric.cxx
  ../Metrics/ClosestDistanceSurfaceToSurfaceMetric.cxx
  ../Metrics/HausdorffDistanceSurfaceToSurfaceMetric.cxx
  ../Metrics/CurrentsSurfaceToSurfaceMetric.cxx
)
add_executable(randomizeLabel randomizeLabel.cxx)
add_executable(validateLabelImages validateLabelImages.cxx)

target_link_libraries(testImageMetrics ${ITK_LIBRARIES} ${VTK_LIBRARIES})
target_link_libraries(testSurfMetrics ${ITK_LIBRARIES} ${VTK_LIBRARIES})
target_link_libraries(randomizeLabel ${ITK_LIBRARIES})
target_link_libraries(validateLabelImages ${ITK_LIBRARIES} ${VTK_LIBRARIES})
