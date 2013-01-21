

#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkImageRegionIteratorWithIndex.h"

#include "itkBinaryThresholdImageFilter.h"
#include "itkMinimumMaximumImageCalculator.h"

#include "RandomDilateImageFilter.h"

#include "itkOutputWindow.h"
#include "itkTextOutput.h"

#include <exception>
#include <iostream>
#include <string>

#include <cstdlib>
#include <ctime>


int
randomizeLabels(int argc, char** argv)
{
  if (argc != 3)
  {
    std::cerr << "Usage: " << argv[0] << " in.mha out.mha" << std::endl;
    return -1;
  }

  srand(time(NULL));

  itk::OutputWindow::SetInstance(itk::TextOutput::New());

  typedef itk::Image<short, 3> ImageType;

  typedef itk::ImageFileReader<ImageType> ReaderType;

  ReaderType::Pointer reader = ReaderType::New();
  reader->SetFileName(argv[1]);
  reader->Update();

  ImageType::Pointer inimg = reader->GetOutput();

  ImageType::Pointer outimg = ImageType::New();
  outimg->CopyInformation(inimg);
  outimg->SetRegions(inimg->GetLargestPossibleRegion());
  outimg->Allocate();
  outimg->FillBuffer(0);

  // Random dilation
  typedef itk::MinimumMaximumImageCalculator<ImageType> MinMaxCalculator;

  MinMaxCalculator::Pointer minmax = MinMaxCalculator::New();
  minmax->SetImage(reader->GetOutput());
  minmax->ComputeMaximum();

  long maxL = minmax->GetMaximum();

  for (long label = 1; label <= maxL; label++)
  {
    std::cout << "Randomization of label " << label << std::endl;

    typedef itk::BinaryThresholdImageFilter<ImageType, ImageType>
      ThresholderType;

    ThresholderType::Pointer thresf = ThresholderType::New();
    thresf->SetInput(reader->GetOutput());
    thresf->SetLowerThreshold(label);
    thresf->SetUpperThreshold(label);
    thresf->Update();

    typedef RandomDilateImageFilter<ImageType, ImageType> RandomizerType;
    RandomizerType::Pointer randz = RandomizerType::New();
    randz->SetInput(thresf->GetOutput());
    randz->SetRadius(2);
    randz->Update();

    typedef itk::ImageRegionIteratorWithIndex<ImageType> IteratorType;
    IteratorType it(randz->GetOutput(), inimg->GetLargestPossibleRegion());

    for (it.GoToBegin(); !it.IsAtEnd(); ++it)
    {
      ImageType::IndexType ind = it.GetIndex();

      if (it.Get() != 0)
      {
        if (outimg->GetPixel(ind) == 0)
          outimg->SetPixel(ind, label);
        else
        {
          double u = rand() / (double)RAND_MAX;
          if (u > 0.5)
            outimg->SetPixel(ind, label);
        }
      }
    }

  }

  // Random deformation
  // TODO

  typedef itk::ImageFileWriter<ImageType> WriterType;
  WriterType::Pointer writer = WriterType::New();
  writer->SetFileName(argv[2]);
  writer->SetInput(outimg);
  writer->UseCompressionOn();
  writer->Update();

  return 0;
}

int
main(int argc, char** argv)
{
  try
  {
    randomizeLabels(argc, argv);
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
