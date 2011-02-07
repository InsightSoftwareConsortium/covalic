
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
typename HistogramImageToImageMetric<TFixedImage,TMovingImage>::MeasureType
HistogramImageToImageMetric<TFixedImage,TMovingImage>
::GetValue(const TransformParametersType& parameters) const

template <class TLabelImage>

DiceOverlapImageToImageMetric<TLabelImage>
::ComputeOverlaps()
{
  if (m_FixedImage.IsNull() || m_MovingImage.IsNull())
    itkExceptionMacro(<< "Need two input classification images");

  // Define iterators
  typedef itk::ImageRegionIterator<FixedImageType> FixedIteratorType;
  typedef itk::ImageRegionIterator<MovingImageType> MovingIteratorType;

  IteratorType it1(m_Input1, m_Input1->GetRequestedRegion());
  IteratorType it2(m_Input2, m_Input2->GetRequestedRegion());

  // Count number of labels from max value in both images
  // also count number of samples
  unsigned int numLabels = 0;
  unsigned int numSamples = 0;

  it1.GoToBegin();
  it2.GoToBegin();
  while (!it1.IsAtEnd() && !it2.IsAtEnd())
  {
    unsigned int r = (unsigned int)it1.Get();
    unsigned int c = (unsigned int)it2.Get();

    if (r != 0 && c != 0)
    {
      if (r > numLabels)
        numLabels = r;

      if (c > numLabels)
        numLabels = c;

      ++numSamples;
    }

    ++it1;
    ++it2;
  }

  DynArray<double> overlaps;
  overlaps.Allocate(numLabels);

  for (unsigned int label = 1; label <= numLabels; label++)
  {
    // Get intersection and individual set sizes
    unsigned int numIntersect = 0;
    unsigned int numA = 0;
    unsigned int numB = 0;

    it1.GoToBegin();
    it2.GoToBegin();
    while (!it1.IsAtEnd())
    {
      unsigned int s = (unsigned int)it1.Get();
      unsigned int t = (unsigned int)it2.Get();

      bool a = (s == label);
      bool b = (t == label);

      if (a && b)
        numIntersect++;

      if (a)
        numA++;

      if (b)
        numB++;
  
      ++it1;
      ++it2;
    }

    // Overlap or similarity coeff is intersect / average
    double avgSize = (numA + numB) / 2.0;
    //double ratio = (double)numIntersect / avgSize;

    double ratio = (double)numIntersect / numB;

    overlaps.Append(ratio);
  }

  return overlaps;
}

#endif
