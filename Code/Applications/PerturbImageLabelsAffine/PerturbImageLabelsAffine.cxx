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
  float sx, float sy, float sz,
  float tx, float ty, float tz,
  float rx, float ry, float rz, bool randomMode)
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
  ScalarType lrx = rx*degsToRads;
  ScalarType lry = ry*degsToRads;
  ScalarType lrz = rz*degsToRads;

  if (randomMode)
  {
    lrx = generator->GetUniformVariate(-rx, rx)*degsToRads;
    lry = generator->GetUniformVariate(-ry, ry)*degsToRads;
    lrz = generator->GetUniformVariate(-rz, rz)*degsToRads;
  }

  // Determine center of rotation
  typedef itk::ImageMomentsCalculator<ImageType> CalculatorType;
  CalculatorType::Pointer moments = CalculatorType::New();
  moments->SetImage(labelImage);
  moments->Compute();

  // Determine scaling factors at random
  float lsx = sx;
  float lsy = sy;
  float lsz = sz;
  
  if (randomMode)
  {  
    float lsx = generator->GetUniformVariate(1/sx, sx);
    float lsy = generator->GetUniformVariate(1/sy, sy);
    float lsz = generator->GetUniformVariate(1/sz, sz);
  }

  // Determine translation distances at random
  float ltx = tx;
  float lty = ty;
  float ltz = tz;
  
  if (randomMode)
  {
    float ltx = generator->GetUniformVariate(-tx, tx);
    float lty = generator->GetUniformVariate(-ty, ty);
    float ltz = generator->GetUniformVariate(-tz, tz);
  }

  // Build affine transform
  typedef itk::CenteredAffineTransform<ScalarType, Dimension> TransformType;
  TransformType::Pointer transform = TransformType::New();
  transform->SetCenter(moments->GetCenterOfGravity());

  // Set the rotation component
  TransformType::OutputVectorType axisX, axisY, axisZ;
  axisX[0] = 1; axisX[1] = 0;  axisX[2] = 0;
  axisY[0] = 0; axisY[1] = 1;  axisY[2] = 0;
  axisZ[0] = 0; axisZ[1] = 0;  axisZ[2] = 1;
  transform->Rotate3D(axisX, lrx);
  transform->Rotate3D(axisY, lry);
  transform->Rotate3D(axisZ, lrz);

  //  Set the scale component
  TransformType::OutputVectorType scaleXYZ;
  scaleXYZ[0] = lsx; scaleXYZ[1] = lsy; scaleXYZ[2] = lsz;	
  transform->Scale(scaleXYZ);

  std::cout<<scaleXYZ;
  
  // Set the translation component
  TransformType::OutputVectorType translateXYZ;
  translateXYZ[0] = ltx; translateXYZ[1] = lty; translateXYZ[2] = ltz;	
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

  // Maybe some error checking would be in order

  try
  {
    if (maxRotationAngle != 0.0) { rx = maxRotationAngle;  ry = maxRotationAngle;  rz = maxRotationAngle; }
    if (maxScaleFactor != 1.0) { sx = maxScaleFactor;  sy = maxScaleFactor;  sz = maxScaleFactor; }
    if (maxTranslation != 0.0) { tx = maxTranslation;  ty = maxTranslation;  tz = maxTranslation; }

    perturbImageLabels(
      inputVolume.c_str(), outputVolume.c_str(),
      1/sx, 1/sy, 1/sz,
      tx, ty, tz,
      rx, ry, rz, randomMode);
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
