
#include "DiceOverlapImageToImageMetric.h"

#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageRegionIteratorWithIndex.h"

#include "itkOutputWindow.h"
#include "itkTextOutput.h"

#include <exception>
#include <iostream>
#include <string>


int
validateDice(const char* fn1, const char* fn2)
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

  typedef DiceOverlapImageToImageMetric<ByteImageType, ByteImageType>
    DiceMetricType;
  DiceMetricType::Pointer diceMetric = DiceMetricType::New();
  diceMetric->SetFixedImage(Amask);
  diceMetric->SetMovingImage(Bmask);

  std::cout << "Dice(A,B) = " <<  diceMetric->GetValue() << std::endl;

  return 0;

}

int
main(int argc, char** argv)
{
  if (argc != 3)
  {
    std::cerr << "Usage: " << argv[0] << " label_img1 label_img2" << std::endl;
    return -1;
  }

  try
  {
    validateDice(argv[1], argv[2]);
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
