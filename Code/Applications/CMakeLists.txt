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

find_package(VTK COMPONENTS
  vtkCommonCore
  vtkFiltersCore
  vtkFiltersSources
  vtkIOLegacy
  vtkIOPLY
)


include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/../Converters
  ${CMAKE_CURRENT_SOURCE_DIR}/../Applications/Common
  ${CMAKE_CURRENT_SOURCE_DIR}/../Metrics
)

# Metrics for binary images. Given label image, generates multiple outputs for each label
add_subdirectory(ValidateImageDice)
add_subdirectory(ValidateImageJaccard)
add_subdirectory(ValidateImagePPV)
add_subdirectory(ValidateImageSensitivity)
add_subdirectory(ValidateImageSpecificity)
add_subdirectory(ValidateImageAveDist)
add_subdirectory(ValidateImageHausdorffDist)

# Initial Input Image Validation/debugging metric
add_subdirectory(ValidateInputImage)

# Metrics for label images
add_subdirectory(ValidateImageKappa)

# Metrics for surfaces
add_subdirectory(ValidateSurfaceCurrents)
add_subdirectory(ValidateSurfaceHausdorff)

# Image perturbations
add_subdirectory(PerturbImageLabelsMorphology)
add_subdirectory(PerturbImageLabelsMorphology2Das3D)
add_subdirectory(PerturbImageLabelsAffine)
add_subdirectory(PerturbImageLabelsBSpline)
add_subdirectory(PerturbImageLabelsBSpline2Das3D)
