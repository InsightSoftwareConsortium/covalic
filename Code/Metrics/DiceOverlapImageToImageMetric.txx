
#ifndef _DiceOverlapImageToImageMetric_txx
#define _DiceOverlapImageToImageMetric_txx

#include "DiceOverlapImageToImageMetric.h"

#include "itkImageRegionIterator.h"

template <class TFixedImage, class TMovingImage>
DiceOverlapImageToImageMetric<TFixedImage, TMovingImage>
::DiceOverlapImageToImageMetric()
{

}

template <class TFixedImage, class TMovingImage>
DiceOverlapImageToImageMetric<TFixedImage, TMovingImage>
::~DiceOverlapImageToImageMetric()
{

}

template <class TFixedImage, class TMovingImage>
typename DiceOverlapImageToImageMetric<TFixedImage,TMovingImage>::MeasureType
DiceOverlapImageToImageMetric<TFixedImage, TMovingImage>
::GetValue() const
{
  if (Superclass::m_FixedImage.IsNull() || Superclass::m_MovingImage.IsNull())
    itkExceptionMacro(<< "Need two input classification images");

  // Define iterators
  typedef itk::ImageRegionConstIterator<FixedImageType> FixedIteratorType;
  typedef itk::ImageRegionConstIterator<MovingImageType> MovingIteratorType;

  FixedIteratorType fixedIt(Superclass::m_FixedImage, Superclass::m_FixedImage->GetRequestedRegion());
  MovingIteratorType movingIt(Superclass::m_MovingImage, Superclass::m_MovingImage->GetRequestedRegion());

  // Get intersection and individual set sizes
  unsigned int numIntersect = 0;
  unsigned int numA = 0;
  unsigned int numB = 0;

  fixedIt.GoToBegin();
  movingIt.GoToBegin();
  while (!fixedIt.IsAtEnd() && !movingIt.IsAtEnd())
  {
    unsigned int r = (unsigned int)fixedIt.Get();
    unsigned int c = (unsigned int)movingIt.Get();

    bool a = (r != 0);
    bool b = (c != 0);
      
    if (a && b)
      numIntersect++;

    if (a)
      numA++;

    if (b)
      numB++;
  
    ++fixedIt;
    ++movingIt;
  }

  // Overlap or similarity coeff is intersect / average
  double avgSize = (numA + numB) / 2.0 + 1e-20;

  return (double)numIntersect / avgSize;
}

#endif
