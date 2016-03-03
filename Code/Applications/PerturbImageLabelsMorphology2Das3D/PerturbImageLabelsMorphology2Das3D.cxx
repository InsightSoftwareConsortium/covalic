
#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkImageRegionIteratorWithIndex.h"

#include "itkFlatStructuringElement.h"
#include "itkBinaryDilateImageFilter.h"
#include "itkBinaryErodeImageFilter.h"
#include "itkSignedMaurerDistanceMapImageFilter.h"

#include "itkMersenneTwisterRandomVariateGenerator.h"

#include "itkExtractImageFilter.h"
#include "itkOrientImageFilter.h"

#include "itkOutputWindow.h"
#include "itkTextOutput.h"

#include <exception>
#include <iostream>
#include <string>
#include <vector>

#include <cmath>

#include "PerturbImageLabelsMorphology2Das3DCLP.h"

// Each pixel is a grey value represented by a float
typedef unsigned char PixelType;
typedef double ScalarType;
 
// The 2d or 3d image types
typedef itk::Image<PixelType, 3> VolumeImageType;
typedef itk::Image<PixelType, 2> SliceImageType;
typedef itk::Image<float, 2> FloatImageType;

// A reader for 3D images
typedef itk::ImageFileReader<VolumeImageType> VolumeImageReaderType;
// A writer for 2D images
typedef itk::ImageFileWriter<SliceImageType> SliceImageReaderType;

// A filter for taking a 2D slice of a 3D image
typedef itk::ExtractImageFilter<VolumeImageType, SliceImageType> FilterType2D;

// A writer for 3D images
typedef itk::ImageFileWriter<VolumeImageType> VolumeImageWriter;

// Extracts a 2D slice from a 3D image
SliceImageType::Pointer Extract2DSlice(VolumeImageType::Pointer image, int plane, int slice) 
{	// Begin Extract2DSlice

    	FilterType2D::Pointer filter = FilterType2D::New();
   
    	VolumeImageType::RegionType inputRegion = image->GetLargestPossibleRegion();
   
    	VolumeImageType::SizeType size = inputRegion.GetSize();
		  size[plane] = 0;
   
    	VolumeImageType::IndexType start = inputRegion.GetIndex();
    	const unsigned int sliceNumber = slice;
    	start[plane] = sliceNumber;
   
    	VolumeImageType::RegionType desiredRegion;
    	desiredRegion.SetSize(size);
    	desiredRegion.SetIndex(start);
   
    	filter->SetExtractionRegion(desiredRegion);
      filter->SetDirectionCollapseToIdentity();
     	filter->SetInput(image);

    	SliceImageType::Pointer img = filter->GetOutput();
    	img->Update();
	
    	return img;

}	// End Extract2DSlice

template <unsigned int Dimension>
int
perturbImageLabels(
  const char* inputFN, const char* outputFN, int numIterations, int radius)
{

  // Create the reader
	VolumeImageReaderType::Pointer imageReader3D = VolumeImageReaderType::New();

	// Sets the filename for the image to read
	imageReader3D->SetFileName(inputFN);
	// Update the reader -- force it to read the image
	imageReader3D->Update();

	// Get the output of the reader
	VolumeImageType::Pointer labelImage = imageReader3D->GetOutput();

  // To extract a 2D slice from the 3D image
	SliceImageType::Pointer sliceImage;
  int dimToSlice = 0;

  // Figure out which dimension is zero
  VolumeImageType::SizeType imageSize = labelImage->GetLargestPossibleRegion().GetSize();
    
  std::cout<<imageSize<<std::endl;

  if (imageSize[0] == 1) { dimToSlice = 0; }
  else if (imageSize[1] == 1) {dimToSlice == 1; }
  else if (imageSize[2] == 1) { dimToSlice == 1; }
  else { std::cerr << "Image appears to be 3D, try running PerturbImageLabelsMorphology." << std::endl; return -1;} 

  sliceImage = Extract2DSlice(labelImage, dimToSlice, 0);

  typedef itk::ImageRegionIteratorWithIndex<SliceImageType> LabelIteratorType;

  // Get maximum label
  int maxLabel = 0;

  LabelIteratorType mIt(sliceImage, sliceImage->GetLargestPossibleRegion());
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

  typedef itk::FlatStructuringElement<2> StructuringElementType;

  StructuringElementType::RadiusType elementRadius;
  elementRadius.Fill(radius);

  typedef itk::BinaryDilateImageFilter<SliceImageType, SliceImageType, StructuringElementType> BinaryDilateImageFilterType;
  typedef itk::BinaryErodeImageFilter<SliceImageType, SliceImageType, StructuringElementType> BinaryErodeImageFilterType;

  for (int iter = 0; iter < numIterations; iter++)
  {
    // Get label counts from current image state
    std::vector<int> labelCounts(maxLabel+1, 0);

    int totalCount = 0;

    LabelIteratorType it(sliceImage, sliceImage->GetLargestPossibleRegion());
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

    StructuringElementType structuringElement = StructuringElementType::Ball(elementRadius);

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
    typedef itk::BinaryThresholdImageFilter<SliceImageType, SliceImageType> BinaryThresholdImageFilterType;
 
    BinaryThresholdImageFilterType::Pointer thresholdFilter = BinaryThresholdImageFilterType::New();
    thresholdFilter->SetInput(sliceImage);
    thresholdFilter->SetLowerThreshold(pickedLabel);
    thresholdFilter->SetUpperThreshold(pickedLabel);
    thresholdFilter->SetInsideValue(1);
    thresholdFilter->SetOutsideValue(0);
    thresholdFilter->Update();

    SliceImageType::Pointer maskImage = thresholdFilter->GetOutput();

    // Flip coin to determine whether to dilate or erode
    float u = generator->GetUniformVariate(0.0, 1.0);

    if (u < 0.5)
    {
      // Dilate
      std::cout << "Dilating label " << pickedLabel << std::endl;

      BinaryDilateImageFilterType::Pointer dilateFilter = BinaryDilateImageFilterType::New();
      dilateFilter->SetInput(maskImage);
      dilateFilter->SetKernel(structuringElement);
      dilateFilter->SetDilateValue(1);
      dilateFilter->Update();

      maskImage = dilateFilter->GetOutput();

      // Update label image, replace label with pickedLabel if mask is on
      for (it.GoToBegin(); !it.IsAtEnd(); ++it)
      {
        SliceImageType::IndexType ind = it.GetIndex();
        if (maskImage->GetPixel(ind) != 0)
          sliceImage->SetPixel(ind, pickedLabel);
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
        BinaryThresholdImageFilterType::Pointer thresholdFilter =  BinaryThresholdImageFilterType::New();
        thresholdFilter->SetInput(sliceImage);
        thresholdFilter->SetLowerThreshold(i);
        thresholdFilter->SetUpperThreshold(i);
        thresholdFilter->SetInsideValue(1);
        thresholdFilter->SetOutsideValue(0);
        thresholdFilter->Update();

        typedef  itk::SignedMaurerDistanceMapImageFilter< SliceImageType, FloatImageType > SignedMaurerDistanceMapImageFilterType;
        SignedMaurerDistanceMapImageFilterType::Pointer distanceMapImageFilter = SignedMaurerDistanceMapImageFilterType::New();
        distanceMapImageFilter->SetInput(thresholdFilter->GetOutput());
        distanceMapImageFilter->Update();

        labelDistanceMaps.push_back(distanceMapImageFilter->GetOutput());
      }

      BinaryErodeImageFilterType::Pointer erodeFilter = BinaryErodeImageFilterType::New();
      erodeFilter->SetInput(maskImage);
      erodeFilter->SetKernel(structuringElement);
      erodeFilter->SetErodeValue(1);
      erodeFilter->Update();

      SliceImageType::Pointer erodedMaskImage = erodeFilter->GetOutput();

      // Update label image by comparing original mask with eroded mask, then
      // filling with nearest label for eroded regions
      for (it.GoToBegin(); !it.IsAtEnd(); ++it)
      {
        SliceImageType::IndexType ind = it.GetIndex();

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

  SliceImageType::SizeType sliceSize = sliceImage->GetLargestPossibleRegion().GetSize();

  std::cout<<sliceSize<<std::endl;

  for (uint k=0; k<sliceSize[0]; k++)
  {
	  for (uint l=0; l<sliceSize[1]; l++)
	  {

		  SliceImageType::IndexType pixelIndex;
		  pixelIndex[0] = k;  pixelIndex[1] = l;

		  SliceImageType::PixelType pixelValue = sliceImage->GetPixel( pixelIndex ); 

		  VolumeImageType::IndexType nrrdPixelIndex;

      if (dimToSlice == 0) { nrrdPixelIndex[0] = 0;  nrrdPixelIndex[1] = k;  nrrdPixelIndex[2] = l; }
      else if (dimToSlice == 1) { nrrdPixelIndex[0] = k;  nrrdPixelIndex[1] = 0;  nrrdPixelIndex[2] = l; }
		  else { nrrdPixelIndex[0] = k;  nrrdPixelIndex[1] = k;  nrrdPixelIndex[2] = 0; }

		  labelImage->SetPixel(nrrdPixelIndex, pixelValue);
	  }
  }

  VolumeImageWriter::Pointer VolumeImageWriter = VolumeImageWriter::New();
  VolumeImageWriter->SetFileName(outputFN);
  VolumeImageWriter->SetInput(labelImage);
  VolumeImageWriter->Update();

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
    perturbImageLabels<3>(
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
