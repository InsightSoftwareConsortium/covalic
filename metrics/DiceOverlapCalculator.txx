
#ifndef _DiceOverlapCalculator_txx

#include "itkImageRegionIterator.h"

#include "vnl/vnl_matrix.h"

#include "DiceOverlapCalculator.h"

#include "muException.h"

template <class TLabelImage>
DiceOverlapCalculator<TLabelImage>
::DiceOverlapCalculator()
{

}

template <class TLabelImage>
DiceOverlapCalculator<TLabelImage>
::~DiceOverlapCalculator()
{

}

template <class TLabelImage>
void
DiceOverlapCalculator<TLabelImage>
::SetFirstInput(TLabelImage* img)
{
  m_Input1 = img;
}

template <class TLabelImage>
void
DiceOverlapCalculator<TLabelImage>
::SetSecondInput(TLabelImage* img)
{
  m_Input2 = img;
}

template <class TLabelImage>
DynArray<double>
DiceOverlapCalculator<TLabelImage>
::ComputeOverlaps()
{
  if (m_Input1.IsNull() || m_Input2.IsNull())
    muExceptionMacro(<< "Need two input classification images");

  // Define iterators
  typedef itk::ImageRegionIterator<LabelImageType> IteratorType;

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
