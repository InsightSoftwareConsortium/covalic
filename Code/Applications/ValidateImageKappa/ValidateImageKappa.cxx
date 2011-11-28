
#include "CohenKappaImageToImageMetric.h"

#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageRegionIteratorWithIndex.h"

#include "itkOutputWindow.h"
#include "itkTextOutput.h"

#include <exception>
#include <iostream>
#include <string>

#include "ValidateImageKappaCLP.h"


int
validateImageKappa(const char* fn1, const char* fn2, const char* outFile)
{

  itk::OutputWindow::SetInstance(itk::TextOutput::New());

  typedef itk::Image<unsigned char, 3> ByteImageType;

  typedef itk::ImageFileReader<ByteImageType> ReaderType;

  ByteImageType::Pointer Amask;
  {
    ReaderType::Pointer reader = ReaderType::New();
    reader->SetFileName(fn1);
    reader->Update();
    Amask = reader->GetOutput();
  }

  ByteImageType::Pointer Bmask;
  {
    ReaderType::Pointer reader = ReaderType::New();
    reader->SetFileName(fn2);
    reader->Update();
    Bmask = reader->GetOutput();
  }

  typedef CohenKappaImageToImageMetric<ByteImageType, ByteImageType>
    CohenKappaMetricType;
  CohenKappaMetricType::Pointer kappaMetric = CohenKappaMetricType::New();
  kappaMetric->SetFixedImage(Amask);
  kappaMetric->SetMovingImage(Bmask);

  std::ofstream outputfile;
  outputfile.open(outFile, std::ios::out);
  outputfile << "Kappa(A,B) = " <<  kappaMetric->GetValue() << std::endl;
  outputfile.close();

  return 0;

}

int
main(int argc, char** argv)
{
  PARSE_ARGS;

  try
  {
    validateImageKappa(
      inputVolume1.c_str(), inputVolume2.c_str(), outputFile.c_str());
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
