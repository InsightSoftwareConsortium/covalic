
#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkImageRegionIteratorWithIndex.h"

#include "itkFlatStructuringElement.h"
#include "itkBinaryDilateImageFilter.h"
#include "itkBinaryErodeImageFilter.h"
#include "itkSignedMaurerDistanceMapImageFilter.h"

#include "itkMersenneTwisterRandomVariateGenerator.h"

#include "itkOutputWindow.h"
#include "itkTextOutput.h"

#include <exception>
#include <iostream>
#include <string>
#include <vector>

#include <cmath>

#include "PerturbImageLabelsMorphologyCLP.h"


int
perturbImageLabels(
  const char* inputFN, const char* outputFN, int numIterations, int radius)
{

  itk::OutputWindow::SetInstance(itk::TextOutput::New());

  typedef itk::Image<unsigned short, 3> LabelImageType;
  typedef itk::Image<float, 3> FloatImageType;

  typedef itk::ImageFileReader<LabelImageType> ReaderType;

  ReaderType::Pointer reader = ReaderType::New();
  reader->SetFileName(inputFN);
  reader->Update();

  LabelImageType::Pointer labelImage = reader->GetOutput();

  typedef itk::ImageRegionIteratorWithIndex<LabelImageType> LabelIteratorType;

  // Get maximum label
  int maxLabel = 0;

  LabelIteratorType mIt(labelImage, labelImage->GetLargestPossibleRegion());
  for (mIt.GoToBegin(); !mIt.IsAtEnd(); ++mIt)
  {
    if (mIt.Get() > maxLabel)
      maxLabel = mIt.Get();
  }

  std::cout << "Max label = " << maxLabel << std::endl;

  if (maxLabel == 0)
    return 0;

  typedef itk::Statistics::MersenneTwisterRandomVariateGenerator GeneratorType;
  GeneratorType::Pointer generator = GeneratorType::New();
  generator->Initialize();

  typedef itk::FlatStructuringElement<3> StructuringElementType;

  StructuringElementType::RadiusType elementRadius;
  elementRadius.Fill(radius);

   typedef itk::BinaryDilateImageFilter<LabelImageType, LabelImageType, StructuringElementType>
    BinaryDilateImageFilterType;
   typedef itk::BinaryErodeImageFilter<LabelImageType, LabelImageType, StructuringElementType>
    BinaryErodeImageFilterType;

  for (int iter = 0; iter < numIterations; iter++)
  {
    // Get label counts from current image state
    std::vector<int> labelCounts(maxLabel+1, 0);

    int totalCount = 0;

    LabelIteratorType it(labelImage, labelImage->GetLargestPossibleRegion());
    for (it.GoToBegin(); !it.IsAtEnd(); ++it)
    {
      int k = it.Get();

      labelCounts[k]++;

      if (k != 0)
        totalCount++;
    }

    std::cout << "Label counts = ";
    for (int i = 0; i < labelCounts.size(); i++)
      std::cout << labelCounts[i] << " ";
    std::cout << std::endl;

    if (totalCount == 0)
      break;

    // Pick label to modify from [1, maxLabel]
    int pickedLabel = 0;

    while (true)
    {
      //pickedLabel = generator->GetIntegerVariate(maxLabel-1) + 1;
      pickedLabel = generator->GetIntegerVariate(maxLabel);
      if (labelCounts[pickedLabel] != 0)
        break;
    }

    // Randomly pick element type
    //for (int dim = 0; dim < 3; dim++)
    //  elementRadius[dim] = generator->GetIntegerVariate(2) + 1;

    StructuringElementType structuringElement =
      StructuringElementType::Ball(elementRadius);

/*
    int t = generator->GetIntegerVariate(3);
    if (t == 0)
      structuringElement = StructuringElementType::Ball(elementRadius);
    else if (t == 1)
      structuringElement = StructuringElementType::Box(elementRadius);
    else if (t == 2)
      structuringElement = StructuringElementType::Cross(elementRadius);
    else
      structuringElement = StructuringElementType::Polygon(elementRadius, 16);

    std::cout << "Element type " << t << std::endl;
*/

    StructuringElementType::Iterator kernel_it;
    for (kernel_it = structuringElement.Begin(); kernel_it != structuringElement.End(); ++kernel_it)
    {
      float c = generator->GetUniformVariate(0.0, 1.0);
      if (*kernel_it && c > 0.5)
        *kernel_it = false;
    }

    std::cout << "Element radius " << elementRadius << std::endl;

    // Compute binary mask of picked label
    typedef itk::BinaryThresholdImageFilter<LabelImageType, LabelImageType>
      BinaryThresholdImageFilterType;
 
    BinaryThresholdImageFilterType::Pointer thresholdFilter =
      BinaryThresholdImageFilterType::New();
    thresholdFilter->SetInput(labelImage);
    thresholdFilter->SetLowerThreshold(pickedLabel);
    thresholdFilter->SetUpperThreshold(pickedLabel);
    thresholdFilter->SetInsideValue(1);
    thresholdFilter->SetOutsideValue(0);
    thresholdFilter->Update();

    LabelImageType::Pointer maskImage = thresholdFilter->GetOutput();

    // Flip coin to determine whether to dilate or erode
    float u = generator->GetUniformVariate(0.0, 1.0);

    if (u < 0.5)
    {
      // Dilate
      std::cout << "Dilating label " << pickedLabel << std::endl;

      BinaryDilateImageFilterType::Pointer dilateFilter =
        BinaryDilateImageFilterType::New();
      dilateFilter->SetInput(maskImage);
      dilateFilter->SetKernel(structuringElement);
      dilateFilter->SetDilateValue(1);
      dilateFilter->Update();

      maskImage = dilateFilter->GetOutput();

      // Update label image, replace label with pickedLabel if mask is on
      for (it.GoToBegin(); !it.IsAtEnd(); ++it)
      {
        LabelImageType::IndexType ind = it.GetIndex();
        if (maskImage->GetPixel(ind) != 0)
          labelImage->SetPixel(ind, pickedLabel);
          //it.Set(pickedLabel);
      }

    }
    else
    {
      // Erode
      std::cout << "Eroding label " << pickedLabel << std::endl;

      // Compute distance maps for filling in eroded values later
      // by assigning missing values in original mask to nearest label
      std::vector<FloatImageType::Pointer> labelDistanceMaps;
      for (int i = 0; i <= maxLabel; i++)
      {
        BinaryThresholdImageFilterType::Pointer thresholdFilter =
          BinaryThresholdImageFilterType::New();
        thresholdFilter->SetInput(labelImage);
        thresholdFilter->SetLowerThreshold(i);
        thresholdFilter->SetUpperThreshold(i);
        thresholdFilter->SetInsideValue(1);
        thresholdFilter->SetOutsideValue(0);
        thresholdFilter->Update();

        typedef  itk::SignedMaurerDistanceMapImageFilter< LabelImageType, FloatImageType > SignedMaurerDistanceMapImageFilterType;
        SignedMaurerDistanceMapImageFilterType::Pointer distanceMapImageFilter =
          SignedMaurerDistanceMapImageFilterType::New();
        distanceMapImageFilter->SetInput(thresholdFilter->GetOutput());
        distanceMapImageFilter->Update();

        labelDistanceMaps.push_back(distanceMapImageFilter->GetOutput());
      }

      BinaryErodeImageFilterType::Pointer erodeFilter =
        BinaryErodeImageFilterType::New();
      erodeFilter->SetInput(maskImage);
      erodeFilter->SetKernel(structuringElement);
      erodeFilter->SetErodeValue(1);
      erodeFilter->Update();

      LabelImageType::Pointer erodedMaskImage = erodeFilter->GetOutput();

      // Update label image by comparing original mask with eroded mask, then
      // filling with nearest label for eroded regions
      for (it.GoToBegin(); !it.IsAtEnd(); ++it)
      {
        LabelImageType::IndexType ind = it.GetIndex();

        //if (erodedMaskImage->GetPixel(ind) != 0)
        //  continue;

        if (maskImage->GetPixel(ind) == erodedMaskImage->GetPixel(ind))
          continue;

        int closestLabel = pickedLabel;
        float closestDistance = 1e+20;

        for (int i = 0; i <= maxLabel; i++)
        {
          if (i == pickedLabel)
            continue;

          float d = fabsf(labelDistanceMaps[i]->GetPixel(ind));
          if (d < closestDistance)
          {
            closestDistance = d;
            closestLabel = i;
          }
        }

        it.Set(closestLabel);
      }

    } // else u
  } // for iter

  // Write perturbed label image to file
  typedef itk::ImageFileWriter<LabelImageType> WriterType;

  WriterType::Pointer writer = WriterType::New();
  writer->SetInput(labelImage);
  writer->SetFileName(outputFN);
  writer->Update();

  return 0;

}

int
main(int argc, char** argv)
{
  PARSE_ARGS;

  if (iterations < 1)
  {
    std::cerr << "Number of iterations must be >= 1" << std::endl;
    return -1;
  }

  if (radius < 1)
  {
    std::cerr << "Kernel radius must be >= 1" << std::endl;
    return -1;
  }

  try
  {
    perturbImageLabels(
      inputVolume.c_str(), outputVolume.c_str(), iterations, radius);
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
