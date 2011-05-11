
#ifndef _AverageDistanceImageToImageMetric_txx
#define _AverageDistanceImageToImageMetric_txx

#include "itkBSplineInterpolateImageFunction.h"
#include "itkDiscreteGaussianImageFilter.h"
#include "itkImageRegionConstIteratorWithIndex.h"
#include "itkLinearInterpolateImageFunction.h"
#include "itkSignedDanielssonDistanceMapImageFilter.h"

#include "itkVTKImageExport.h"

#include "vtkContourFilter.h"
#include "vtkIdList.h"
#include "vtkImageData.h"
#include "vtkImageImport.h"
#include "vtkPointData.h"
#include "vtkPolyData.h"
#include "vtkSmartPointer.h"

#include "AverageDistanceImageToImageMetric.h"


template <class TFixedImage, class TMovingImage>
AverageDistanceImageToImageMetric<TFixedImage, TMovingImage>
::AverageDistanceImageToImageMetric()
{
  m_DoBlurring = false;
}

template <class TFixedImage, class TMovingImage>
AverageDistanceImageToImageMetric<TFixedImage, TMovingImage>
::~AverageDistanceImageToImageMetric()
{

}

template <class TFixedImage, class TMovingImage>
double
AverageDistanceImageToImageMetric<TFixedImage, TMovingImage>
::Compute3DNonSymmetricDistance(
  const TFixedImage* img1, const TMovingImage* img2) const
{
  typedef itk::Image<float, FixedImageType::ImageDimension> FloatImageType;

  typedef itk::SignedDanielssonDistanceMapImageFilter<
    FixedImageType, FloatImageType> DistanceMapFilterType;

  typedef itk::DiscreteGaussianImageFilter<
    FloatImageType, FloatImageType> BlurFilterType;

  // Compute distance transform
  typename FloatImageType::Pointer distMap2;
  {
    typename DistanceMapFilterType::Pointer distanceMapFilter =
      DistanceMapFilterType::New();

    distanceMapFilter->InsideIsPositiveOff();
    distanceMapFilter->SetInput(img2);
    distanceMapFilter->SquaredDistanceOff();
    distanceMapFilter->UseImageSpacingOn();

    distanceMapFilter->Update();

    if (!m_DoBlurring)
    {
      distMap2 = distanceMapFilter->GetDistanceMap();
    }
    else
    {
      typename BlurFilterType::Pointer blurf = BlurFilterType::New();
      blurf->SetInput(distanceMapFilter->GetDistanceMap());
      blurf->SetVariance(0.01);
      blurf->Update();

      distMap2 = blurf->GetOutput();
    }
  }

  //typedef itk::LinearInterpolateImageFunction<FloatImageType, double>
  typedef itk::BSplineInterpolateImageFunction<FloatImageType, double>
    InterpolatorType;
  typename InterpolatorType::Pointer distInterp2 = InterpolatorType::New();
  distInterp2->SetInputImage(distMap2);
  distInterp2->SetSplineOrder(3);

  // Do contour detection
  typename FloatImageType::Pointer contourMask = FloatImageType::New();
  contourMask->CopyInformation(img1);
  contourMask->SetRegions(img1->GetLargestPossibleRegion());
  contourMask->Allocate();
  contourMask->FillBuffer(0);

  typedef itk::ImageRegionConstIteratorWithIndex<FixedImageType>
    FixedIteratorType;
  FixedIteratorType it1(img1, img1->GetLargestPossibleRegion());

  for (it1.GoToBegin(); !it1.IsAtEnd(); ++it1)
  {
    if (it1.Get() == 0)
      continue;

    contourMask->SetPixel(it1.GetIndex(), 1.0);
  }

  typedef itk::VTKImageExport<FloatImageType> ITKExportType;
  typename ITKExportType::Pointer itkexport = ITKExportType::New();
  itkexport->SetInput(contourMask);
  itkexport->Update();

  // See InsightApplications/Auxialiary/vtk/itkImageToVTKImageFilter
  vtkSmartPointer<vtkImageImport> vtkimport =
    vtkSmartPointer<vtkImageImport>::New();
  vtkimport->SetUpdateInformationCallback(
    itkexport->GetUpdateInformationCallback());
  vtkimport->SetPipelineModifiedCallback(
    itkexport->GetPipelineModifiedCallback());
  vtkimport->SetWholeExtentCallback(itkexport->GetWholeExtentCallback());
  vtkimport->SetSpacingCallback(itkexport->GetSpacingCallback());
  vtkimport->SetOriginCallback(itkexport->GetOriginCallback());
  vtkimport->SetScalarTypeCallback(itkexport->GetScalarTypeCallback());
  vtkimport->SetNumberOfComponentsCallback(itkexport->GetNumberOfComponentsCallback());
  vtkimport->SetPropagateUpdateExtentCallback(itkexport->GetPropagateUpdateExtentCallback());
  vtkimport->SetUpdateDataCallback(itkexport->GetUpdateDataCallback());
  vtkimport->SetDataExtentCallback(itkexport->GetDataExtentCallback());
  vtkimport->SetBufferPointerCallback(itkexport->GetBufferPointerCallback());
  vtkimport->SetCallbackUserData(itkexport->GetCallbackUserData());

  vtkSmartPointer<vtkContourFilter> contourf =
    vtkSmartPointer<vtkContourFilter>::New();
  contourf->SetInput(vtkimport->GetOutput());
  contourf->SetNumberOfContours(1);
  contourf->SetValue(0, 1.0);
  contourf->ComputeNormalsOff();
  contourf->ComputeGradientsOff();

  contourf->Update();

  vtkSmartPointer<vtkPolyData> contourPD = contourf->GetOutput();
  if (contourPD->GetNumberOfPoints() == 0)
  {
    itkExceptionMacro(<< "No boundary points detected");
    return 0.0;
  }

  double sumD = 0;
  for (vtkIdType k = 0; k < contourPD->GetNumberOfPoints(); k++)
  {
    double x[3];
    contourPD->GetPoint(k, x);

    typename FloatImageType::PointType p;
    p[0] = x[0];
    p[1] = x[1];
    p[2] = x[2];

    if (!distInterp2->IsInsideBuffer(p))
    {
      continue;
    }

    double d = distInterp2->Evaluate(p);

    sumD += fabs(d);
  }

  double meanD = sumD / contourPD->GetNumberOfPoints();

  return meanD;
}

template <class TFixedImage, class TMovingImage>
typename AverageDistanceImageToImageMetric<TFixedImage, TMovingImage>::MeasureType
AverageDistanceImageToImageMetric<TFixedImage, TMovingImage>
::GetValue() const
{
  if (Superclass::m_FixedImage.IsNull() || Superclass::m_MovingImage.IsNull())
    itkExceptionMacro(<< "Need two input classification images");

  // TODO:
  // if (ImageDimension == 2) marching squares
  // else if (ImageDimension == 3) marching cubes
  // else do generic boundary detection

  unsigned int dim = FixedImageType::GetImageDimension();

  if (dim == 3)
  {
    double d12 = this->Compute3DNonSymmetricDistance(
      Superclass::m_FixedImage, Superclass::m_MovingImage);
    double d21 = this->Compute3DNonSymmetricDistance(
      Superclass::m_MovingImage, Superclass::m_FixedImage);
    return 0.5 * (d12 + d21);
  }

  itkExceptionMacro(<< "Not implemented");

  return 0;
}

#endif
