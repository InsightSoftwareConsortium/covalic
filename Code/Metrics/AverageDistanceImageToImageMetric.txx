
#ifndef _AverageDistanceImageToImageMetric_txx
#define _AverageDistanceImageToImageMetric_txx

#include "itkBSplineInterpolateImageFunction.h"
#include "itkDiscreteGaussianImageFilter.h"
#include "itkImageRegionConstIteratorWithIndex.h"
#include "itkLinearInterpolateImageFunction.h"
//#include "itkSignedDanielssonDistanceMapImageFilter.h"
#include "itkSignedMaurerDistanceMapImageFilter.h"

#include "itkVTKImageExport.h"

#include "vtkContourFilter.h"
#include "vtkIdList.h"
#include "vtkImageData.h"
#include "vtkImageImport.h"
#include "vtkPointData.h"
#include "vtkPolyData.h"
#include "vtkSmartPointer.h"
#include "vtkTriangleFilter.h"

#include "vnl/vnl_math.h"

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

  //typedef itk::SignedDanielssonDistanceMapImageFilter<
  typedef itk::SignedMaurerDistanceMapImageFilter<
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
      //distMap2 = distanceMapFilter->GetDistanceMap();
      distMap2 = distanceMapFilter->GetOutput();
    }
    else
    {
      typename FloatImageType::SpacingType spacing = img1->GetSpacing();

      double minSpacing = spacing[0];
      for (unsigned int dim = 0; dim < FixedImageType::ImageDimension; dim++)
        if (spacing[dim] < minSpacing)
          minSpacing = spacing[dim];

      typename BlurFilterType::Pointer blurf = BlurFilterType::New();
      //blurf->SetInput(distanceMapFilter->GetDistanceMap());
      blurf->SetInput(distanceMapFilter->GetOutput());
      blurf->SetVariance(0.5 * minSpacing);
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

  vtkSmartPointer<vtkTriangleFilter> trif = vtkSmartPointer<vtkTriangleFilter>::New();
  trif->SetInput(contourf->GetOutput());
  trif->PassVertsOff();
  trif->PassLinesOff();
  trif->Update();

  vtkSmartPointer<vtkPolyData> boundaryPD = trif->GetOutput();

  boundaryPD->BuildLinks();

  if (boundaryPD->GetNumberOfCells() == 0)
  {
    itkExceptionMacro(<< "No boundary points detected");
    return 0.0;
  }

  double sumD = 0;
  double sumArea = 0;

  for (vtkIdType k = 0; k < boundaryPD->GetNumberOfCells(); k++)
  {
    // Compute centroid
    double c[3];
    for (int d = 0; d < 3; d++)
      c[d] = 0;

    vtkIdType nPts = 0;
    vtkIdType* ptIds = 0;
    boundaryPD->GetCellPoints(k, nPts, ptIds);

    if (nPts != 3)
      itkExceptionMacro(<< "Non triangle cell detected: " << nPts);


    double x0[3];
    boundaryPD->GetPoint(ptIds[0], x0);
    double x1[3];
    boundaryPD->GetPoint(ptIds[1], x1);
    double x2[3];
    boundaryPD->GetPoint(ptIds[2], x2);

    for (int d = 0; d < 3; d++)
    {
      c[d] += (x0[d] + x1[d] + x2[d]) / 3.0;
    }

    for (int d = 0; d < 3; d++)
    {
      x1[d] = x1[d] - x0[d];
      x2[d] = x2[d] - x0[d];
    }

    // Compute area
    double crossp[3];
    crossp[0] = (x1[1] * x2[2] - x1[2] * x2[1]) / 2.0;
    crossp[1] = (x1[2] * x2[0] - x1[0] * x2[2]) / 2.0;
    crossp[2] = (x1[0] * x2[1] - x1[1] * x2[0]) / 2.0;

    double area = 0;
    for (int d = 0; d < 3; d++)
      area += crossp[d] * crossp[d];
    area /= 2.0;

    // Compute distance
    typename FloatImageType::PointType p;
    p[0] = c[0];
    p[1] = c[1];
    p[2] = c[2];

    if (!distInterp2->IsInsideBuffer(p))
    {
      continue;
    }

    double d = distInterp2->Evaluate(p);

    sumD += vnl_math_abs(d) * area;
    sumArea += area;
  }

  double meanD = sumD / sumArea;

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
