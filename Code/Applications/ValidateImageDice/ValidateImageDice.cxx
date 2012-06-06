
#include "DiceOverlapImageToImageMetric.h"
#include "MultipleBinaryImageMetricsCalculator.h"

#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageRegionIteratorWithIndex.h"

#include "itkOutputWindow.h"
#include "itkTextOutput.h"

#include <exception>
#include <iostream>
#include <string>

#include "ValidateImageDiceCLP.h"


int
validateImageDice(const char* fn1, const char* fn2, const char* outFile)
{

  itk::OutputWindow::SetInstance(itk::TextOutput::New());

  typedef itk::Image<unsigned short, 3> ImageType;

  typedef itk::ImageFileReader<ImageType> ReaderType;

  ImageType::Pointer truthImg;
  {
    ReaderType::Pointer reader = ReaderType::New();
    reader->SetFileName(fn1);
    reader->Update();
    truthImg = reader->GetOutput();
  }

  ImageType::Pointer testImg;
  {
    ReaderType::Pointer reader = ReaderType::New();
    reader->SetFileName(fn2);
    reader->Update();
    testImg = reader->GetOutput();
  }

  std::ofstream outputfile;
  outputfile.open(outFile, std::ios::out);

  typedef DiceOverlapImageToImageMetric<ImageType, ImageType>
    DiceMetricType;

  typedef MultipleBinaryImageMetricsCalculator<ImageType, ImageType, DiceMetricType>
    DiceCalculatorType;
  DiceCalculatorType::Pointer calc = DiceCalculatorType::New();
  calc->SetFixedImage(truthImg);
  calc->SetMovingImage(testImg);
  calc->Update();
  for (unsigned int i = 0; i < calc->GetNumberOfValues(); i++)
    outputfile << "Dice(" << "A_" << i+1 << ", B_" << i+1 << ") = " << calc->GetValue(i) << std::endl;

  outputfile.close();

  return 0;

}

int
main(int argc, char** argv)
{
  PARSE_ARGS;

  try
  {
    validateImageDice(
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
