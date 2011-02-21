
#ifndef _JaccardOverlapImageToImageMetric_txx

#include "JaccardOverlapImageToImageMetric.h"

#include "itkImageRegionIterator.h"

template <class TFixedImage, class TMovingImage>
JaccardOverlapImageToImageMetric<TFixedImage, TMovingImage>
::JaccardOverlapImageToImageMetric()
{

}

template <class TFixedImage, class TMovingImage>
JaccardOverlapImageToImageMetric<TFixedImage, TMovingImage>
::~JaccardOverlapImageToImageMetric()
{

}

template <class TFixedImage, class TMovingImage>
typename JaccardOverlapImageToImageMetric<TFixedImage,TMovingImage>::MeasureType
JaccardOverlapImageToImageMetric<TFixedImage, TMovingImage>
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
  unsigned int numUnion = 0;

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

    if (a || b)
      numUnion++;

    ++fixedIt;
    ++movingIt;
  }

  return (double)numIntersect / (numUnion + 1e-20);
}

#endif
