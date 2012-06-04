
#ifndef _SpecificityImageToImageMetric_txx
#define _SpecificityImageToImageMetric_txx

#include "SpecificityImageToImageMetric.h"

#include "itkImageRegionIterator.h"

template <class TFixedImage, class TMovingImage>
SpecificityImageToImageMetric<TFixedImage, TMovingImage>
::SpecificityImageToImageMetric()
{

}

template <class TFixedImage, class TMovingImage>
SpecificityImageToImageMetric<TFixedImage, TMovingImage>
::~SpecificityImageToImageMetric()
{

}

template <class TFixedImage, class TMovingImage>
typename SpecificityImageToImageMetric<TFixedImage,TMovingImage>::MeasureType
SpecificityImageToImageMetric<TFixedImage, TMovingImage>
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
  unsigned int numTrueNegatives = 0;
  unsigned int numFalsePositives = 0;

  fixedIt.GoToBegin();
  movingIt.GoToBegin();
  while (!fixedIt.IsAtEnd() && !movingIt.IsAtEnd())
  {
    unsigned int r = (unsigned int)fixedIt.Get();
    unsigned int c = (unsigned int)movingIt.Get();

    bool a = (r != 0);
    bool b = (c != 0);
      
    if (!a && !b)
      numTrueNegatives++;

    if (!a && b)
      numFalsePositives++;

    ++fixedIt;
    ++movingIt;
  }

  return numTrueNegatives / (numTrueNegatives + numFalsePositives + 1e-20);
}

#endif
