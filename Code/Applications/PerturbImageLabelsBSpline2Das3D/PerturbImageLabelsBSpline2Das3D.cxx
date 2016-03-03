#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkImageRegionIteratorWithIndex.h"

#include "itkMersenneTwisterRandomVariateGenerator.h"

#include "itkResampleImageFilter.h"
#include "itkNearestNeighborInterpolateImageFunction.h"

#include "itkExtractImageFilter.h"
#include "itkOrientImageFilter.h"

#include "itkBSplineTransform.h"
#include "itkBSplineTransformInitializer.h"

#include <exception>
#include <iostream>
#include <string>
#include <cmath>

#include "PerturbImageLabelsBSpline2Das3DCLP.h"

// Each pixel is a grey value represented by a float
typedef unsigned char PixelType;
typedef double ScalarType;
 
// The 2d or 3d image types
typedef itk::Image<PixelType, 3> VolumeImageType;
typedef itk::Image<PixelType, 2> SliceImageType;

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

int
perturbImageLabels(
  const char* inputFN, const char* outputFN, 
  float normalMean, float normalVariance, unsigned int gridNodes)
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
  else { std::cerr << "Image appears to be 3D, try running PerturbImageLabelsBSpline." << std::endl; return -1;} 

  sliceImage = Extract2DSlice(labelImage, dimToSlice, 0);
  
  // Setup random number generator
  typedef itk::Statistics::MersenneTwisterRandomVariateGenerator GeneratorType;
  GeneratorType::Pointer generator = GeneratorType::New();
  generator->Initialize();

  // A cubic spline
  const unsigned int SplineOrder = 3;

  // Build BSpline transform
  typedef itk::BSplineTransform<ScalarType, 2, SplineOrder> TransformType;
  TransformType::Pointer outputBSplineTransform = TransformType::New();

  // Initialize the BSpline transform
  typedef itk::BSplineTransformInitializer< TransformType, SliceImageType> InitializerType;
  InitializerType::Pointer transformInitializer = InitializerType::New();
  TransformType::MeshSizeType meshSize;
  meshSize.Fill( gridNodes - SplineOrder );
  transformInitializer->SetTransform( outputBSplineTransform );
  transformInitializer->SetImage( sliceImage );
  transformInitializer->SetTransformDomainMeshSize( meshSize );
  transformInitializer->InitializeTransform();
 
  // Set random parameters
  typedef TransformType::ParametersType ParametersType;
  const unsigned int numberOfParameters = outputBSplineTransform->GetNumberOfParameters();
  ParametersType parameters( numberOfParameters );
  
  // Generate random parameters from a normal distrubtion
  for (int i=0; i<numberOfParameters; i++)
  {
    parameters[i] = generator->GetNormalVariate(normalMean, normalVariance);
  }

  outputBSplineTransform->SetParameters( parameters );
 
  // Resample image by applying BSpline transform and nearest neighbor interpolatoin
  typedef itk::NearestNeighborInterpolateImageFunction< SliceImageType, ScalarType> InterpolatorType;
  InterpolatorType::Pointer interpolator = InterpolatorType::New();

  typedef itk::ResampleImageFilter<SliceImageType, SliceImageType> ResampleImageFilterType;
  ResampleImageFilterType::Pointer resample = ResampleImageFilterType::New();
  resample->SetInput(sliceImage);
  resample->SetReferenceImage(sliceImage);
  resample->UseReferenceImageOn();
  resample->SetSize(sliceImage->GetLargestPossibleRegion().GetSize());
  
  resample->SetInterpolator(interpolator);
  resample->SetTransform(outputBSplineTransform);
  resample->Update();

  sliceImage = resample->GetOutput();

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

  if (normalVariance <= 0.0)
  {
    std::cerr << "Variance must be > 0" << std::endl;
    return -1;
  }

  if (gridNodes < 4)
  {
    std::cerr << "Number of grid nodes must be >= 4" << std::endl;
    return -1;
  }

  try
  {
    
    perturbImageLabels(
      inputVolume.c_str(), outputVolume.c_str(),
      normalMean, normalVariance, gridNodes);

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
