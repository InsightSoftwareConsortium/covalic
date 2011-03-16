cmake_minimum_required(VERSION 2.6)

project(validation_apps)

include (${CMAKE_ROOT}/Modules/FindITK.cmake)
if (USE_ITK_FILE)
  include(${USE_ITK_FILE})
endif (USE_ITK_FILE)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/../Converters
  ${CMAKE_CURRENT_SOURCE_DIR}/../Metrics
)

add_executable(validateDice validateDice.cxx)
add_executable(validateJaccard validateJaccard.cxx)

target_link_libraries(validateDice ${ITK_LIBRARIES})
target_link_libraries(validateJaccard ${ITK_LIBRARIES})
