cmake_minimum_required(VERSION 2.6)

project(validation)

include (${CMAKE_ROOT}/Modules/FindITK.cmake)
if (USE_ITK_FILE)
  include(${USE_ITK_FILE})
endif (USE_ITK_FILE)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/../Converters
  ${CMAKE_CURRENT_SOURCE_DIR}/../Metrics
)

add_executable(testMetrics testMetrics.cxx)

target_link_libraries(testMetrics ${ITK_LIBRARIES})
