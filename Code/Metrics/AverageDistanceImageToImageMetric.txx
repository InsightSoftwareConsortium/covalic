
#ifndef _AverageDistanceImageToImageMetric_txx
#define _AverageDistanceImageToImageMetric_txx

#include "itkBSplineInterpolateImageFunction.h"
#include "itkDiscreteGaussianImageFilter.h"
#include "itkImageRegionConstIteratorWithIndex.h"
#include "itkLinearInterpolateImageFunction.h"
#include "itkSignedMaurerDistanceMapImageFilter.h"

#include "itkBinaryBallStructuringElement.h"
#include "itkMorphologicalGradientImageFilter.h"

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
::ComputeNonSymmetricDistance(
  const TFixedImage* img1, const TMovingImage* img2) const
{
  typedef itk::Image<float, FixedImageType::ImageDimension> FloatImageType;

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
      blurf->SetInput(distanceMapFilter->GetOutput());
      blurf->SetVariance(1.5 * minSpacing);
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

  // Detect boundary via morphological gradient
  typedef itk::BinaryBallStructuringElement<FixedImagePixelType, TFixedImage::ImageDimension>
    StructElementType;
  typedef
    itk::MorphologicalGradientImageFilter<FixedImageType, FixedImageType,
      StructElementType> EdgeFilterType;

  StructElementType structel;
  structel.SetRadius(1);
  structel.CreateStructuringElement();

  typename EdgeFilterType::Pointer edgef = EdgeFilterType::New();
  edgef->SetInput(img1);
  edgef->SetKernel(structel);
  edgef->Update();

  FixedImagePointer edgeImg1 = edgef->GetOutput();

  double sumD = 0;
  double numV = 0;

  typedef itk::ImageRegionConstIteratorWithIndex<FixedImageType>
    FixedIteratorType;
  FixedIteratorType it(edgeImg1, edgeImg1->GetLargestPossibleRegion());

  for (it.GoToBegin(); !it.IsAtEnd(); ++it)
  {
    FixedImageIndexType ind = it.GetIndex();

    if (it.Get() == 0 || img1->GetPixel(ind) == 0)
      continue;

    FixedImagePointType p;
    img1->TransformIndexToPhysicalPoint(ind, p);

    if (!distInterp2->IsInsideBuffer(p))
      continue;

    sumD += vnl_math_abs(distInterp2->Evaluate(p));
    numV += 1.0;
  }

  if (numV == 0)
    return vnl_huge_val(1.0);


  return sumD / numV;

}

template <class TFixedImage, class TMovingImage>
typename AverageDistanceImageToImageMetric<TFixedImage, TMovingImage>::MeasureType
AverageDistanceImageToImageMetric<TFixedImage, TMovingImage>
::GetValue() const
{
  if (Superclass::m_FixedImage.IsNull() || Superclass::m_MovingImage.IsNull())
    itkExceptionMacro(<< "Need two input classification images");

  // Handle special case where inputs are zeros
  typedef itk::ImageRegionConstIteratorWithIndex<FixedImageType>
    FixedIteratorType;
  FixedIteratorType it(Superclass::m_FixedImage, Superclass::m_FixedImage->GetLargestPossibleRegion());

  double sumFixed = 0;
  double sumMoving = 0;
  for (it.GoToBegin(); !it.IsAtEnd(); ++it)
  {
    sumFixed += it.Get();
    sumMoving += Superclass::m_MovingImage->GetPixel(it.GetIndex());
  }

  if (sumFixed == 0 || sumMoving == 0)
  {
    if (sumFixed == sumMoving)
      return 0.0;
    else
      return vnl_huge_val(1.0);
  }

  double d12 = this->ComputeNonSymmetricDistance(
    Superclass::m_FixedImage, Superclass::m_MovingImage);

  if (vnl_math_isinf(d12))
    return vnl_huge_val(1.0);

  double d21 = this->ComputeNonSymmetricDistance(
    Superclass::m_MovingImage, Superclass::m_FixedImage);

  if (vnl_math_isinf(d21))
    return vnl_huge_val(1.0);

  return 0.5 * (d12 + d21);

}

#endif
