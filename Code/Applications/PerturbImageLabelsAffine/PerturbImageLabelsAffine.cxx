
#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkImageRegionIteratorWithIndex.h"

#include "itkMersenneTwisterRandomVariateGenerator.h"

#include "itkOutputWindow.h"
#include "itkTextOutput.h"

#include <exception>
#include <iostream>
#include <string>

#include <cmath>

#include "PerturbImageLabelsAffineCLP.h"


int
perturbImageLabels(
  const char* inputFN, const char* outputFN,
  float maxRotationAngle, float maxScaleFactor)
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

  // Setup random number generator
  typedef itk::Statistics::MersenneTwisterRandomVariateGenerator GeneratorType;
  GeneratorType::Pointer generator = GeneratorType::New();
  generator->Initialize();

  // Determine rotation angles at random
  float rx = generator->GetUniformVariate(0.0, maxRotationAngle);
  float ry = generator->GetUniformVariate(0.0, maxRotationAngle);
  float rz = generator->GetUniformVariate(0.0, maxRotationAngle);
//TODO: convert to radians?

  // Determine center of rotation
// TODO: centroid of label mass?

  // Determine scaling factors at random
  float minScaleFactor = 1.0 / maxScaleFactor;

  float sx = generator->GetUniformVariate(minScaleFactor, maxScaleFactor);
  float sy = generator->GetUniformVariate(minScaleFactor, maxScaleFactor);
  float sz = generator->GetUniformVariate(minScaleFactor, maxScaleFactor);

  // Build affine transform
//TODO

  // Resample image by applying affine transform
//TODO

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
      maxRotationAngle, maxScaleFactor);
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
