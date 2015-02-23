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

#include "itkOutputWindow.h"
#include "itkTextOutput.h"

#include <exception>
#include <iostream>
#include <string>


int
validateLabelImages(const char* fixedfn, const char* movingfn)
{
  typedef itk::Image<unsigned short, 3> ImageType;

  typedef itk::ImageFileReader<ImageType> ReaderType;

  ReaderType::Pointer freader = ReaderType::New();
  freader->SetFileName(fixedfn);
  freader->Update();
  ImageType::Pointer fixedImage = freader->GetOutput();

  ReaderType::Pointer mreader = ReaderType::New();
  mreader->SetFileName(movingfn);
  mreader->Update();
  ImageType::Pointer movingImage = mreader->GetOutput();

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
      std::cout << "Ppv" << i+1 << "=" << calc->GetValue(i) << std::endl;
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
  std::cout << "Kap" << kappa->GetValue() << std::endl;

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
