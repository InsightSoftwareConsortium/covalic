/*
 * Runs all image comparison metrics in the suite, outputting the resulting
 * metric one per line to stdout in the form:
 *
 * <metric_name>=<value>
 */

#include "DiceOverlapImageToImageMetric.h"
#include "JaccardOverlapImageToImageMetric.h"
#include "SensitivityImageToImageMetric.h"
#include "SpecificityImageToImageMetric.h"
#include "PositivePredictiveValueImageToImageMetric.h"

#include "MultipleBinaryImageMetricsCalculator.h"

#include "CohenKappaImageToImageMetric.h"

#include "AverageDistanceImageToImageMetric.h"
#include "HausdorffDistanceImageToImageMetric.h"

#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageRegionIterator.h"

#include "itkOutputWindow.h"
#include "itkTextOutput.h"

#include <exception>
#include <iostream>
#include <set>
#include <string>

typedef unsigned short PixelType;
typedef itk::Image<PixelType, 3> ImageType;

/**
 * Validate that the given image contains only two labels. This is implemented by
 * iterating over the pixels in the image and ensuring that the number of distinct
 * labels is no more than two.
 */
bool validateLabelCount(const ImageType::Pointer& image)
{
  itk::ImageRegionConstIterator<ImageType> itr(image, image->GetRequestedRegion());
  std::set<PixelType> distinctValues;

  itr.GoToBegin();
  while (!itr.IsAtEnd())
  {
    distinctValues.insert(itr.Get());

    if (distinctValues.size() > 2) {
        return false;
    }
    ++itr;
  }
  return true;
}

int
validateLabelImages(const char* fixedfn, const char* movingfn)
{
  typedef itk::ImageFileReader<ImageType> ReaderType;

  ReaderType::Pointer freader = ReaderType::New();
  freader->SetFileName(fixedfn);
  freader->Update();
  ImageType::Pointer fixedImage = freader->GetOutput();

  ReaderType::Pointer mreader = ReaderType::New();
  mreader->SetFileName(movingfn);
  mreader->Update();
  ImageType::Pointer movingImage = mreader->GetOutput();

  ImageType::SizeType fsize = fixedImage->GetLargestPossibleRegion().GetSize();
  ImageType::SizeType msize = movingImage->GetLargestPossibleRegion().GetSize();

  if (fsize != msize) {
    std::cerr << "Error: Image sizes do not match. Make sure that your image " <<
      "orientation is correct." << std::endl;
    std::cerr << "Fixed size: " << fsize << std::endl;
    std::cerr << "Moving size: " << msize << std::endl;
    return 1;
  }

  if (!validateLabelCount(fixedImage)) {
    std::cerr << "Error: " << fixedfn << " has more than two labels." << std::endl;
    return 1;
  }
  if (!validateLabelCount(movingImage)) {
    std::cerr << "Error: " << movingfn << " has more than two labels." << std::endl;
    return 1;
  }

  // Metrics for binary data
  typedef DiceOverlapImageToImageMetric<ImageType, ImageType>
    DiceMetricType;
  typedef JaccardOverlapImageToImageMetric<ImageType, ImageType>
    JaccardMetricType;
  typedef SpecificityImageToImageMetric<ImageType, ImageType>
    SpecificityMetricType;
  typedef SensitivityImageToImageMetric<ImageType, ImageType>
    SensitivityMetricType;
  typedef PositivePredictiveValueImageToImageMetric<ImageType, ImageType>
    PPVMetricType;
  typedef AverageDistanceImageToImageMetric<ImageType, ImageType>
    AverageDistanceMetricType;
  typedef HausdorffDistanceImageToImageMetric<ImageType, ImageType>
    HausdorffDistanceMetricType;

  // Metrics for label data
  typedef CohenKappaImageToImageMetric<ImageType, ImageType>
    KappaMetricType;

  typedef MultipleBinaryImageMetricsCalculator<ImageType, ImageType, DiceMetricType>
    DiceCalculatorType;
  {
    DiceCalculatorType::Pointer calc = DiceCalculatorType::New();
    calc->SetFixedImage(fixedImage);
    calc->SetMovingImage(movingImage);
    calc->Update();
    for (unsigned int i = 0; i < calc->GetNumberOfValues(); i++)
      std::cout << "Dice" << i+1 << "=" << calc->GetValue(i) << std::endl;
  }

  typedef MultipleBinaryImageMetricsCalculator<ImageType, ImageType, JaccardMetricType>
    JaccardCalculatorType;
  {
    JaccardCalculatorType::Pointer calc = JaccardCalculatorType::New();
    calc->SetFixedImage(fixedImage);
    calc->SetMovingImage(movingImage);
    calc->Update();
    for (unsigned int i = 0; i < calc->GetNumberOfValues(); i++)
      std::cout << "Jac" << i+1 << "=" << calc->GetValue(i) << std::endl;
  }

  typedef MultipleBinaryImageMetricsCalculator<ImageType, ImageType, SpecificityMetricType>
    SpecificityCalculatorType;
  {
    SpecificityCalculatorType::Pointer calc = SpecificityCalculatorType::New();
    calc->SetFixedImage(fixedImage);
    calc->SetMovingImage(movingImage);
    calc->Update();
    for (unsigned int i = 0; i < calc->GetNumberOfValues(); i++)
      std::cout << "Spec" << i+1 << "=" << calc->GetValue(i) << std::endl;
  }

  typedef MultipleBinaryImageMetricsCalculator<ImageType, ImageType, SensitivityMetricType>
    SensitivityCalculatorType;
  {
    SensitivityCalculatorType::Pointer calc = SensitivityCalculatorType::New();
    calc->SetFixedImage(fixedImage);
    calc->SetMovingImage(movingImage);
    calc->Update();
    for (unsigned int i = 0; i < calc->GetNumberOfValues(); i++)
      std::cout << "Sens" << i+1 << "=" << calc->GetValue(i) << std::endl;
  }

  typedef MultipleBinaryImageMetricsCalculator<ImageType, ImageType, PPVMetricType>
    PPVCalculatorType;
  {
    PPVCalculatorType::Pointer calc = PPVCalculatorType::New();
    calc->SetFixedImage(fixedImage);
    calc->SetMovingImage(movingImage);
    calc->Update();
    for (unsigned int i = 0; i < calc->GetNumberOfValues(); i++)
      std::cout << "PPV" << i+1 << "=" << calc->GetValue(i) << std::endl;
  }

  typedef MultipleBinaryImageMetricsCalculator<ImageType, ImageType, AverageDistanceMetricType>
    AverageDistanceCalculatorType;
  {
    AverageDistanceCalculatorType::Pointer calc = AverageDistanceCalculatorType::New();
    calc->SetFixedImage(fixedImage);
    calc->SetMovingImage(movingImage);
    calc->Update();
    for (unsigned int i = 0; i < calc->GetNumberOfValues(); i++)
      std::cout << "Adb" << i+1 << "=" << calc->GetValue(i) << std::endl;
  }

  typedef MultipleBinaryImageMetricsCalculator<ImageType, ImageType, HausdorffDistanceMetricType>
    HausdorffDistanceCalculatorType;
  {
    HausdorffDistanceCalculatorType::Pointer calc = HausdorffDistanceCalculatorType::New();
    calc->SetFixedImage(fixedImage);
    calc->SetMovingImage(movingImage);
    calc->Update();
    for (unsigned int i = 0; i < calc->GetNumberOfValues(); i++)
      std::cout << "Hdb" << i+1 << "=" << calc->GetValue(i) << std::endl;
  }

  KappaMetricType::Pointer kappa = KappaMetricType::New();
  kappa->SetFixedImage(fixedImage);
  kappa->SetMovingImage(movingImage);
  std::cout << "Kap=" << kappa->GetValue() << std::endl;

  return 0;
}

int
main(int argc, char** argv)
{
  if (argc != 3)
  {
    std::cerr << argv[0] << " fixed moving" << std::endl;
    return 1;
  }

  int val(0);
  try
  {
    val = validateLabelImages(argv[1], argv[2]);
  }
  catch (itk::ExceptionObject& e)
  {
    std::cerr << e << std::endl;
    return 1;
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << std::endl;
    return 1;
  }
  catch (std::string& s)
  {
    std::cerr << "Exception: " << s << std::endl;
    return 1;
  }
  catch (...)
  {
    std::cerr << "Unknown exception" << std::endl;
    return 1;
  }

  return val;
}
