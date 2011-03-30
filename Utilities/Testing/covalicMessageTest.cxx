/*=========================================================================

Library:   Covalic

Copyright 2010 Kitware Inc. 28 Corporate Drive,
Clifton Park, NY, 12065, USA.

All rights reserved. 

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

=========================================================================*/
#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif

#include <cstdlib>
#include <covalicMessage.h>

int covalicMessageTest(int argc, char* argv [] ) 
{
  if( argc > 1 )
    {
    std::cerr << "Usage: " << std::endl;
    std::cerr << argv[0] << std::endl;
    return EXIT_FAILURE;
    }
  
  covalic::Message( "This is a debug test", covalic::MessageLevel::Debug );
  covalic::Message( "This is a info test", covalic::MessageLevel::Information );
  covalic::Message( "This is a warning test", covalic::MessageLevel::Warning );
  covalic::Message( "This is a Error test", covalic::MessageLevel::Error );

  covalic::DebugMessage( "Debug2" );
  covalic::InfoMessage( "Info2" );
  covalic::WarningMessage( "Warning2" );
  covalic::ErrorMessage( "Error2" );

  return EXIT_SUCCESS;

}

