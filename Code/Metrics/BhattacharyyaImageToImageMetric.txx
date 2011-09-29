
#ifndef _BhattacharyyaImageToImageMetric_txx

#include "BhattacharyyaImageToImageMetric.h"

#include "itkImageRegionIterator.h"

#include <cmath>

template <class TFixedImage, class TMovingImage>
BhattacharyyaImageToImageMetric<TFixedImage, TMovingImage>
::BhattacharyyaImageToImageMetric()
{

}

template <class TFixedImage, class TMovingImage>
BhattacharyyaImageToImageMetric<TFixedImage, TMovingImage>
::~BhattacharyyaImageToImageMetric()
{

}

template <class TFixedImage, class TMovingImage>
typename BhattacharyyaImageToImageMetric<TFixedImage,TMovingImage>::MeasureType
BhattacharyyaImageToImageMetric<TFixedImage, TMovingImage>
::GetValue() const
{
  if (Superclass::m_FixedImage.IsNull() || Superclass::m_MovingImage.IsNull())
    itkExceptionMacro(<< "Need two input classification images");

  // Define iterators
  typedef itk::ImageRegionConstIterator<FixedImageType> FixedIteratorType;
  typedef itk::ImageRegionConstIterator<MovingImageType> MovingIteratorType;

  FixedIteratorType fixedIt(Superclass::m_FixedImage, Superclass::m_FixedImage->GetRequestedRegion());
  MovingIteratorType movingIt(Superclass::m_MovingImage, Superclass::m_MovingImage->GetRequestedRegion());

  typename FixedImageType::VectorLengthType numComponents =
    Superclass::m_FixedImage->GetVectorLength();

  // Get the Bhattacharyya overlap for the two pdf images
  MeasureType bMetric = 0;

  fixedIt.GoToBegin();
  movingIt.GoToBegin();
  while (!fixedIt.IsAtEnd() && !movingIt.IsAtEnd())
  {
    FixedImagePixelType pa = fixedIt.Get();
    FixedImagePixelType pb = movingIt.Get();

    for (unsigned int c = 0; c < numComponents; c++)
    {
      double pa_c = pa[c];
      double pb_c = pb[c];
      bMetric += sqrt(pa_c * pb_c);
    }
  
    ++fixedIt;
    ++movingIt;
  }

  return bMetric;
}

#endif
