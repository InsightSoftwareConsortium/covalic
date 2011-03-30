##############################################################################
# 
# Library:   Covalic
# 
# Copyright 2010 Kitware Inc. 28 Corporate Drive,
# Clifton Park, NY, 12065, USA.
# 
# All rights reserved. 
# 
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# 
#       http://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# 
##############################################################################

include_regular_expression( "^.*$" )

find_package( Tclsh )
if( TCL_TCLSH )
  add_test( PrintSelf-UtilitiesFilters ${TCL_TCLSH}
              ${Covalic_SOURCE_DIR}/Utilities/PrintSelfCheck.tcl  
              ${Covalic_SOURCE_DIR}/Utilities )
endif( TCL_TCLSH )

set( UTILITIES_TESTS 
      ${Covalic_BINARY_DIR}/${Slicer3_INSTALL_BIN_DIR}/covalicUtilitiesTests )

set( UTILITIES_HEADER_TEST 
      ${Covalic_BINARY_DIR}/${Slicer3_INSTALL_BIN_DIR}/covalicUtilitiesHeaderTest )

set( APPLICATION_PATH ${Covalic_BINARY_DIR}/Utilities )

set( INPUT_DATA ${Covalic_SOURCE_DIR}/Data )
set( ITK_INPUT_DATA ${ITK_DATA_ROOT}/Input )

set( BASELINE ${APPLICATION_PATH}/Testing/Baselines )

set( TEMP ${Covalic_BINARY_DIR}/Temporary )

file( COPY ${INPUT_DATA}/ES0015_Large.mha DESTINATION ${TEMP} )
file( COPY ${INPUT_DATA}/im0001.crop.mha DESTINATION ${TEMP} )

# Test CLI generation
set( covalicCLIHelperFunctionsTest_SOURCE
     covalicCLIHelperFunctionsTest.cxx )
generateclp( covalicCLIHelperFunctionsTest_SOURCE 
             covalicCLIHelperFunctionsTest.xml )
     
# Add test here
set( covalicUtilities_SRCS
  covalicUtilitiesPrintTest.cxx
  covalicMessageTest.cxx 
  covalicCLIFilterWatcherTest.cxx 
  covalicCLIProgressReporterTest.cxx 
  ${covalicCLIHelperFunctionsTest_SOURCE} )

include_directories( ${Covalic_SOURCE_DIR}/Utilities )

add_executable( covalicUtilitiesHeaderTest 
  covalicUtilitiesHeaderTest.cxx )
target_link_libraries( covalicUtilitiesHeaderTest
  ITKIO ITKNumerics ITKBasicFilters )

add_executable( covalicUtilitiesTests
  covalicUtilitiesTests.cxx
  ${covalicUtilities_SRCS} )
target_link_libraries( covalicUtilitiesTests
  ITKIO ITKNumerics ITKBasicFilters )

add_test( covalicUtilitiesHeaderTest 
            ${UTILITIES_HEADER_TEST} )

add_test( covalicUtilitiesPrintTest 
            ${UTILITIES_TESTS}
            covalicUtilitiesPrintTest )

## Add tests here
add_test( covalicMessageTest 
            ${UTILITIES_TESTS}
            covalicMessageTest )

add_test( covalicCLIFilterWatcherTest 
            ${UTILITIES_TESTS}
            covalicCLIFilterWatcherTest ${INPUT_DATA}/Branch.n010.mha )

add_test( covalicCLIHelperFunctionsTest 
            ${UTILITIES_TESTS}
            covalicCLIHelperFunctionsTest ${INPUT_DATA}/Branch.n010.mha )

add_test( covalicCLIProgressReporterTest 
            ${UTILITIES_TESTS}
            covalicCLIProgressReporterTest )

## Image Compare
add_test( ImageCompareCommandTest1
  ${Covalic_BINARY_DIR}/${Slicer3_INSTALL_PLUGINS_BIN_DIR}/ImageCompareCommand
    -t ${TEMP}/ES0015_Large.mha
    -b ${INPUT_DATA}/ES0015_Large.mha )

add_test( ImageCompareCommandTest2
  ${Covalic_BINARY_DIR}/${Slicer3_INSTALL_PLUGINS_BIN_DIR}/ImageCompareCommand
    -n 1000
    -r 1
    -i 50
    -t ${TEMP}/im0001.crop.mha
    -b ${INPUT_DATA}/im0001_n10.crop.mha )

add_test( ImageCompareCommandTest3
  ${Covalic_BINARY_DIR}/${Slicer3_INSTALL_PLUGINS_BIN_DIR}/ImageCompareCommand
     -t ${TEMP}/im0001.mha
     -b ${INPUT_DATA}/im0001_n10.crop.mha )
set_tests_properties( ImageCompareCommandTest3
                      PROPERTIES WILL_FAIL true )

add_test( ImageCompareCommandTest4
  ${Covalic_BINARY_DIR}/${Slicer3_INSTALL_PLUGINS_BIN_DIR}/ImageCompareCommand
     -t ${TEMP}/im0001.crop.mha
     -B 2 ${INPUT_DATA}/im0001_n10.crop.mha ${INPUT_DATA}/im0001.crop.mha )

add_test( ImageCompareCommandTest5
  ${Covalic_BINARY_DIR}/${Slicer3_INSTALL_PLUGINS_BIN_DIR}/ImageCompareCommand
     -t ${TEMP}/im0001.crop.mha
     -B 2 ${INPUT_DATA}/im0001_n10.crop.mha ${INPUT_DATA}/im0001_n20.crop.mha )
set_tests_properties( ImageCompareCommandTest5
                      PROPERTIES WILL_FAIL true )


