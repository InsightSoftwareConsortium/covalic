#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkImageRegionIteratorWithIndex.h"

#include "itkMersenneTwisterRandomVariateGenerator.h"

#include "itkResampleImageFilter.h"
#include "itkNearestNeighborInterpolateImageFunction.h"

#include "itkBSplineTransform.h"
#include "itkBSplineTransformInitializer.h"

#include "itkOutputWindow.h"
#include "itkTextOutput.h"

#include <exception>
#include <iostream>
#include <string>
#include <cmath>

#include "PerturbImageLabelsBSplineCLP.h"

#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkResampleImageFilter.h"


int
perturbImageLabels(
  const char* inputFN, const char* outputFN, 
  float normalMean, float normalVariance, unsigned int gridNodes)
{

  const unsigned int Dimension = 3;
  const unsigned int SplineOrder = 3;
  typedef double ScalarType;

  typedef unsigned char PixelType;
  typedef itk::Image< PixelType, Dimension > ImageType;

  itk::OutputWindow::SetInstance(itk::TextOutput::New());

  typedef itk::ImageFileReader<ImageType> ReaderType;

  ReaderType::Pointer reader = ReaderType::New();
  reader->SetFileName(inputFN);
  reader->Update();

  ImageType::ConstPointer labelImage = reader->GetOutput();

  // Setup random number generator
  typedef itk::Statistics::MersenneTwisterRandomVariateGenerator GeneratorType;
  GeneratorType::Pointer generator = GeneratorType::New();
  generator->Initialize();

  // Build BSpline transform
  typedef itk::BSplineTransform<ScalarType, Dimension, SplineOrder> TransformType;
  TransformType::Pointer outputBSplineTransform = TransformType::New();

  // Initialize the BSpline transform
  typedef itk::BSplineTransformInitializer< TransformType, ImageType> InitializerType;
  InitializerType::Pointer transformInitializer = InitializerType::New();
  TransformType::MeshSizeType meshSize;
  meshSize.Fill( gridNodes - SplineOrder );
  transformInitializer->SetTransform( outputBSplineTransform );
  transformInitializer->SetImage( labelImage );
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
  typedef itk::NearestNeighborInterpolateImageFunction< ImageType, ScalarType> InterpolatorType;
  InterpolatorType::Pointer interpolator = InterpolatorType::New();

  typedef itk::ResampleImageFilter<ImageType, ImageType> ResampleImageFilterType;
  ResampleImageFilterType::Pointer resample = ResampleImageFilterType::New();
  resample->SetInput(labelImage);
  resample->SetReferenceImage(labelImage);
  resample->UseReferenceImageOn();
  resample->SetSize(labelImage->GetLargestPossibleRegion().GetSize());
  
  resample->SetInterpolator(interpolator);
  resample->SetTransform(outputBSplineTransform);

  // Write perturbed label image to file
  typedef itk::ImageFileWriter<ImageType> WriterType;

  WriterType::Pointer writer = WriterType::New();
  writer->SetInput(resample->GetOutput());
  writer->SetFileName(outputFN);
  writer->Update();

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
