
#include "CurrentsSurfaceToSurfaceMetric.h"

#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageRegionIteratorWithIndex.h"

#include "itkOutputWindow.h"
#include "itkTextOutput.h"

#include "vtkSmartPointer.h"
#include "vtkPolyData.h"

#include <exception>
#include <iostream>
#include <string>

#include "ValidateSurfaceCurrentsCLP.h"

#include "surfio.h"


int
validateSurfaceCurrents(
  const char* fn1, const char* fn2, double h, const char* outFile)
{

  itk::OutputWindow::SetInstance(itk::TextOutput::New());

  vtkSmartPointer<vtkPolyData> surf1 = readSurface(fn1);

  vtkSmartPointer<vtkPolyData> surf2 = readSurface(fn2);

  CurrentsSurfaceToSurfaceMetric::Pointer currMetric = CurrentsSurfaceToSurfaceMetric::New();
  currMetric->SetFixedSurface(surf1);
  currMetric->SetMovingSurface(surf2);
  currMetric->SetKernelWidth(h);


  std::ofstream outputfile;
  outputfile.open(outFile, std::ios::out);
  outputfile << "Currents(A,B|h = " << h << ") = "
    <<  currMetric->GetValue() << std::endl;
  outputfile.close();

  return 0;

}

int
main(int argc, char** argv)
{
  PARSE_ARGS;

  try
  {
    validateSurfaceCurrents(
      inputSurface1.c_str(), inputSurface2.c_str(), kernelWidth,
      outputFile.c_str());
  } 
  catch (itk::ExceptionObject& e)
  {
    std::cerr << e << std::endl;
    return -1;
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << std::endl;
    return -1;
  }
  catch (std::string& s)
  {
    std::cerr << "Exception: " << s << std::endl;
    return -1;
  }
  catch (...)
  {
    std::cerr << "Unknown exception" << std::endl;
    return -1;
  }

  return 0;


}
