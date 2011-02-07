
#ifndef _DiceOverlapImageToImageMetric_txx

#include "itkImageRegionIterator.h"

#include "vnl/vnl_matrix.h"

#include "DiceOverlapImageToImageMetric.h"

#include "muException.h"

template <class TLabelImage>
DiceOverlapImageToImageMetric<TLabelImage>
::DiceOverlapImageToImageMetric()
{

}

template <class TLabelImage>
DiceOverlapImageToImageMetric<TLabelImage>
::~DiceOverlapImageToImageMetric()
{

}

template <class TFixedImage, class TMovingImage>
typename DiceOverlapImageToImageMetric<TFixedImage,TMovingImage>::MeasureType
DiceOverlapToImageMetric<TFixedImage,TMovingImage>
::GetValue(const TransformParametersType& parameters) const
{
  if (m_FixedImage.IsNull() || m_MovingImage.IsNull())
    itkExceptionMacro(<< "Need two input classification images");

  // Define iterators
  typedef itk::ImageRegionIterator<FixedImageType> FixedIteratorType;
  typedef itk::ImageRegionIterator<MovingImageType> MovingIteratorType;

  IteratorType fixedIt(m_FixedImage, m_FixedImage->GetRequestedRegion());
  IteratorType movingIt(m_MovingImage, m_MovingImage->GetRequestedRegion());

  // Get intersection and individual set sizes
  unsigned int numIntersect = 0;
  unsigned int numA = 0;
  unsigned int numB = 0;

  fixedIt.GoToBegin();
  movingIt.GoToBegin();
  while (!fixedIt.IsAtEnd() && !movingIt.IsAtEnd())
  {
    unsigned int r = (unsigned int)it1.Get();
    unsigned int c = (unsigned int)it2.Get();

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
  double avgSize = (numA + numB) / 2.0;

  return (double)numIntersect / avgSize;
}

#endif
