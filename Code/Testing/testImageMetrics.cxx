
#include "DiceOverlapImageToImageMetric.h"
#include "JaccardOverlapImageToImageMetric.h"

#include "SensitivityImageToImageMetric.h"
#include "SpecificityImageToImageMetric.h"
#include "PositivePredictiveValueImageToImageMetric.h"

#include "CohenKappaImageToImageMetric.h"

#include "AverageDistanceImageToImageMetric.h"
#include "HausdorffDistanceImageToImageMetric.h"

#include "itkImage.h"
#include "itkImageRegionIteratorWithIndex.h"

#include "itkOutputWindow.h"
#include "itkTextOutput.h"

#include <exception>
#include <iostream>
#include <string>


int
testMetrics()
{

  itk::OutputWindow::SetInstance(itk::TextOutput::New());

  typedef itk::Image<unsigned char, 3> ByteImageType;

  ByteImageType::SizeType size = {{64, 64, 64}};

  ByteImageType::RegionType region;
  region.SetSize(size);

  // Create fake images
  ByteImageType::Pointer Amask = ByteImageType::New();
  Amask->SetRegions(region);
  Amask->Allocate();
  Amask->FillBuffer(0);

  ByteImageType::Pointer Bmask = ByteImageType::New();
  Bmask->SetRegions(region);
  Bmask->Allocate();
  Bmask->FillBuffer(0);

  ByteImageType::Pointer Zmask = ByteImageType::New();
  Zmask->SetRegions(region);
  Zmask->Allocate();
  Zmask->FillBuffer(0);

  ByteImageType::IndexType ind;
  for (ind[2] = 0; ind[2] < (long)size[2]; ind[2]++)
    for (ind[1] = 0; ind[1] < (long)size[1]; ind[1]++)
      for (ind[0] = 0; ind[0] < (long)size[0]; ind[0]++)
      {
        if (ind[2] < 16 || ind[2] > 48)
          continue;
        if (ind[1] < 16 || ind[1] > 48)
          continue;
        if (ind[0] < 16 || ind[0] > 48)
          continue;
        Amask->SetPixel(ind, 1);

        if (ind[0] < 32)
          continue;
        Bmask->SetPixel(ind, 1);

        if (ind[0] > 32)
        {
          Amask->SetPixel(ind, 2);
          Bmask->SetPixel(ind, 1);
        }
      }


  typedef DiceOverlapImageToImageMetric<ByteImageType, ByteImageType>
    DiceMetricType;
  DiceMetricType::Pointer diceMetric = DiceMetricType::New();
  diceMetric->SetFixedImage(Amask);
  diceMetric->SetMovingImage(Bmask);

  std::cout << "Dice(A,B) = " <<  diceMetric->GetValue() << std::endl;

  typedef JaccardOverlapImageToImageMetric<ByteImageType, ByteImageType>
    JaccardMetricType;
  JaccardMetricType::Pointer jaccMetric = JaccardMetricType::New();
  jaccMetric->SetFixedImage(Amask);
  jaccMetric->SetMovingImage(Bmask);

  std::cout << "Jaccard(A,B) = " <<  jaccMetric->GetValue() << std::endl;

  typedef SensitivityImageToImageMetric<ByteImageType, ByteImageType>
    SensitivityMetricType;
  SensitivityMetricType::Pointer sensMetric = SensitivityMetricType::New();
  sensMetric->SetFixedImage(Amask);
  sensMetric->SetMovingImage(Bmask);

  std::cout << "Sensitivity(A,B) = " <<  sensMetric->GetValue() << std::endl;

  typedef SpecificityImageToImageMetric<ByteImageType, ByteImageType>
    SpecificityMetricType;
  SpecificityMetricType::Pointer specMetric = SpecificityMetricType::New();
  specMetric->SetFixedImage(Amask);
  specMetric->SetMovingImage(Bmask);

  std::cout << "Specificity(A,B) = " <<  specMetric->GetValue() << std::endl;

  typedef PositivePredictiveValueImageToImageMetric<ByteImageType, ByteImageType>
    PositivePredictiveValueMetricType;
  PositivePredictiveValueMetricType::Pointer precMetric = PositivePredictiveValueMetricType::New();
  precMetric->SetFixedImage(Amask);
  precMetric->SetMovingImage(Bmask);

  std::cout << "PositivePredictiveValue(A,B) = " <<  precMetric->GetValue() << std::endl;

  typedef CohenKappaImageToImageMetric<ByteImageType, ByteImageType>
    KappaMetricType;
  KappaMetricType::Pointer kappaMetric = KappaMetricType::New();
  kappaMetric->SetFixedImage(Amask);
  kappaMetric->SetMovingImage(Bmask);

  std::cout << "Kappa(A,B) = " << kappaMetric->GetValue() << std::endl;

  typedef AverageDistanceImageToImageMetric<ByteImageType, ByteImageType>
    AveDistanceMetricType;
  AveDistanceMetricType::Pointer aveDistMetric = AveDistanceMetricType::New();

  aveDistMetric->SetFixedImage(Amask);
  aveDistMetric->SetMovingImage(Bmask);
  std::cout << "AveDist(A,B) = " << aveDistMetric->GetValue() << std::endl;

  aveDistMetric->SetFixedImage(Amask);
  aveDistMetric->SetMovingImage(Amask);
  std::cout << "AveDist(A,A) = " << aveDistMetric->GetValue() << std::endl;

  aveDistMetric->SetFixedImage(Amask);
  aveDistMetric->SetMovingImage(Zmask);
  std::cout << "AveDist(A,Z) = " << aveDistMetric->GetValue() << std::endl;

  typedef HausdorffDistanceImageToImageMetric<ByteImageType, ByteImageType>
    HausdorffDistanceMetricType;
  HausdorffDistanceMetricType::Pointer hDistMetric = HausdorffDistanceMetricType::New();

  hDistMetric->SetFixedImage(Amask);
  hDistMetric->SetMovingImage(Bmask);
  std::cout << "HausdorffDist(A,B) = " << hDistMetric->GetValue() << std::endl;

  hDistMetric->SetFixedImage(Amask);
  hDistMetric->SetMovingImage(Amask);
  std::cout << "HausdorffDist(A,A) = " << hDistMetric->GetValue() << std::endl;

  hDistMetric->SetFixedImage(Amask);
  hDistMetric->SetMovingImage(Zmask);
  std::cout << "HausdorffDist(A,Z) = " << hDistMetric->GetValue() << std::endl;

  return 0;

}

int
main(int argc, char** argv)
{
  try
  {
    testMetrics();
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
