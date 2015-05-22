#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkImageRegionIteratorWithIndex.h"

#include "itkMersenneTwisterRandomVariateGenerator.h"

#include "itkCenteredAffineTransform.h"
#include "itkResampleImageFilter.h"
#include "itkNearestNeighborInterpolateImageFunction.h"
#include "itkImageMomentsCalculator.h"

#include "itkOutputWindow.h"
#include "itkTextOutput.h"

#include <exception>
#include <iostream>
#include <string>
#include <cmath>

#include "PerturbImageLabelsAffineCLP.h"

#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkAffineTransform.h"
#include "itkResampleImageFilter.h"
#include "itkWindowedSincInterpolateImageFunction.h"

# define PI 3.14159265




int
perturbImageLabels(
  const char* inputFN, const char* outputFN,
  float maxRotationAngle, float maxScaleFactor, float maxTranslation)
{

  const unsigned int Dimension = 3;
  typedef double ScalarType;

  typedef unsigned char PixelType;
  typedef itk::Image< PixelType, Dimension > ImageType;

  itk::OutputWindow::SetInstance(itk::TextOutput::New());

  typedef itk::ImageFileReader<ImageType> ReaderType;

  ReaderType::Pointer reader = ReaderType::New();
  reader->SetFileName(inputFN);
  reader->Update();

  ImageType::ConstPointer labelImage = reader->GetOutput();

  typedef itk::ImageRegionIteratorWithIndex<ImageType> LabelIteratorType;

  // Setup random number generator
  typedef itk::Statistics::MersenneTwisterRandomVariateGenerator GeneratorType;
  GeneratorType::Pointer generator = GeneratorType::New();
  generator->Initialize();

  // Determine rotation angles at random
  ScalarType degsToRads = (PI/180.0);
  ScalarType rx = generator->GetUniformVariate(-maxRotationAngle, maxRotationAngle)*degsToRads;
  ScalarType ry = generator->GetUniformVariate(-maxRotationAngle, maxRotationAngle)*degsToRads;
  ScalarType rz = generator->GetUniformVariate(-maxRotationAngle, maxRotationAngle)*degsToRads;

  // Determine center of rotation
  typedef itk::ImageMomentsCalculator<ImageType> CalculatorType;
  CalculatorType::Pointer moments = CalculatorType::New();
  moments->SetImage(labelImage);
  moments->Compute();

  // Determine scaling factors at random
  float minScaleFactor = 1.0 / maxScaleFactor;
  float sx = generator->GetUniformVariate(minScaleFactor, maxScaleFactor);
  float sy = generator->GetUniformVariate(minScaleFactor, maxScaleFactor);
  float sz = generator->GetUniformVariate(minScaleFactor, maxScaleFactor);

  // Determine translation distances at random
  float tx = generator->GetUniformVariate(-maxTranslation, maxTranslation);
  float ty = generator->GetUniformVariate(-maxTranslation, maxTranslation);
  float tz = generator->GetUniformVariate(-maxTranslation, maxTranslation);

  // Build affine transform
  typedef itk::CenteredAffineTransform<ScalarType, Dimension> TransformType;
  TransformType::Pointer transform = TransformType::New();
  transform->SetCenter(moments->GetCenterOfGravity());

  // Set the rotation component
  TransformType::OutputVectorType axisX, axisY, axisZ;
  axisX[0] = 1; axisX[1] = 0;  axisX[2] = 0;
  axisY[0] = 0; axisY[1] = 1;  axisY[2] = 0;
  axisZ[0] = 0; axisZ[1] = 0;  axisZ[2] = 1;
  transform->Rotate3D(axisX, rx);
  transform->Rotate3D(axisY, ry);
  transform->Rotate3D(axisZ, rz);

  //  Set the scale component
  TransformType::OutputVectorType scaleXYZ;
  scaleXYZ[0] = sx; scaleXYZ[1] = sy; scaleXYZ[2] = sz;	
  transform->Scale(scaleXYZ);
  
  // Set the translation component
  TransformType::OutputVectorType translateXYZ;
  translateXYZ[0] = tx; translateXYZ[1] = ty; translateXYZ[2] = tz;	
  transform->Translate(translateXYZ);

  // Resample image by applying affine transform and nearest neighbor interpolatoin
  typedef itk::NearestNeighborInterpolateImageFunction< ImageType, ScalarType> InterpolatorType;
  InterpolatorType::Pointer interpolator = InterpolatorType::New();

  typedef itk::ResampleImageFilter<ImageType, ImageType> ResampleImageFilterType;
  ResampleImageFilterType::Pointer resample = ResampleImageFilterType::New();
  resample->SetInput(labelImage);
  resample->SetReferenceImage(labelImage);
  resample->UseReferenceImageOn();
  resample->SetSize(labelImage->GetLargestPossibleRegion().GetSize());
  
  resample->SetInterpolator(interpolator);
  resample->SetTransform(transform);

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

  if (maxRotationAngle < 0.0)
  {
    std::cerr << "Max rotation angle must be > 0" << std::endl;
    return -1;
  }

  if (maxScaleFactor <= 1.0)
  {
    std::cerr << "Max scaling factor must be > 1" << std::endl;
    return -1;
  }

  try
  {
    perturbImageLabels(
      inputVolume.c_str(), outputVolume.c_str(),
      maxRotationAngle, maxScaleFactor, maxTranslation);
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
