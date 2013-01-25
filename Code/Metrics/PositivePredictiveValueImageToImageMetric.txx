
#ifndef _PositivePredictiveValueImageToImageMetric_txx
#define _PositivePredictiveValueImageToImageMetric_txx

#include "PositivePredictiveValueImageToImageMetric.h"

#include "itkImageRegionIterator.h"

template <class TFixedImage, class TMovingImage>
PositivePredictiveValueImageToImageMetric<TFixedImage, TMovingImage>
::PositivePredictiveValueImageToImageMetric()
{

}

template <class TFixedImage, class TMovingImage>
PositivePredictiveValueImageToImageMetric<TFixedImage, TMovingImage>
::~PositivePredictiveValueImageToImageMetric()
{

}

template <class TFixedImage, class TMovingImage>
typename PositivePredictiveValueImageToImageMetric<TFixedImage,TMovingImage>::MeasureType
PositivePredictiveValueImageToImageMetric<TFixedImage, TMovingImage>
::GetValue() const
{
  if (Superclass::m_FixedImage.IsNull() || Superclass::m_MovingImage.IsNull())
    itkExceptionMacro(<< "Need two input classification images");

  // Define iterators
  typedef itk::ImageRegionConstIterator<FixedImageType> FixedIteratorType;
  typedef itk::ImageRegionConstIterator<MovingImageType> MovingIteratorType;

  FixedIteratorType fixedIt(Superclass::m_FixedImage, Superclass::m_FixedImage->GetRequestedRegion());
  MovingIteratorType movingIt(Superclass::m_MovingImage, Superclass::m_MovingImage->GetRequestedRegion());

  // Get count of true negatives and false positives
  unsigned int numTruePositives = 0;
  unsigned int numFalsePositives = 0;

  fixedIt.GoToBegin();
  movingIt.GoToBegin();
  while (!fixedIt.IsAtEnd() && !movingIt.IsAtEnd())
  {
    unsigned int r = (unsigned int)fixedIt.Get();
    unsigned int c = (unsigned int)movingIt.Get();

    bool a = (r != 0);
    bool b = (c != 0);
      
    if (a && b)
      numTruePositives++;

    if (!a && b)
      numFalsePositives++;

    ++fixedIt;
    ++movingIt;
  }

  return numTruePositives / (numTruePositives + numFalsePositives + 1e-20);
}

#endif
