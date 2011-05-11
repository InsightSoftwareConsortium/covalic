
#ifndef _CohenKappaImageToImageMetric_txx
#define _CohenKappaImageToImageMetric_txx

#include "CohenKappaImageToImageMetric.h"

#include "itkImageRegionConstIterator.h"

#include "vnl/vnl_matrix.h"

template <class TFixedImage, class TMovingImage>
CohenKappaImageToImageMetric<TFixedImage, TMovingImage>
::CohenKappaImageToImageMetric()
{

}

template <class TFixedImage, class TMovingImage>
CohenKappaImageToImageMetric<TFixedImage, TMovingImage>
::~CohenKappaImageToImageMetric()
{

}

template <class TFixedImage, class TMovingImage>
typename CohenKappaImageToImageMetric<TFixedImage, TMovingImage>::MeasureType
CohenKappaImageToImageMetric<TFixedImage, TMovingImage>
::GetValue() const
{
  if (Superclass::m_FixedImage.IsNull() || Superclass::m_MovingImage.IsNull())
    itkExceptionMacro(<< "Need two input classification images");

  // Define iterators
  typedef itk::ImageRegionConstIterator<FixedImageType> FixedIteratorType;
  typedef itk::ImageRegionConstIterator<MovingImageType> MovingIteratorType;

  FixedIteratorType fixedIt(Superclass::m_FixedImage, Superclass::m_FixedImage->GetRequestedRegion());
  MovingIteratorType movingIt(Superclass::m_MovingImage, Superclass::m_MovingImage->GetRequestedRegion());

  // Count number of labels from max value in both images
  // also count number of samples
  unsigned int numLabels = 0;
  unsigned int numSamples = 0;

  fixedIt.GoToBegin();
  movingIt.GoToBegin();
  while (!fixedIt.IsAtEnd() && !movingIt.IsAtEnd())
  {
    unsigned int r = (unsigned int)fixedIt.Get();
    unsigned int c = (unsigned int)movingIt.Get();

    if (r != 0 && c != 0)
    {
      if (r > numLabels)
        numLabels = r;

      if (c > numLabels)
        numLabels = c;

      ++numSamples;
    }

    ++fixedIt;
    ++movingIt;
  }

  // Fill in matrix that counts agreements/disagreements
  vnl_matrix<unsigned int> countMatrix(numLabels, numLabels);

  fixedIt.GoToBegin();
  movingIt.GoToBegin();
  while (!fixedIt.IsAtEnd() && !movingIt.IsAtEnd())
  {
    unsigned int r = (unsigned int)fixedIt.Get();
    unsigned int c = (unsigned int)movingIt.Get();

    if (r != 0 && c != 0)
    {
      countMatrix(r-1, c-1) += 1;
    }

    ++fixedIt;
    ++movingIt;
  }

  // Process the matrix
  unsigned int sumAgreements = 0;
  for (unsigned int k = 0; k < numLabels; k++)
    sumAgreements += countMatrix(k, k);

  double sumEF = 0;
  for (unsigned int k = 0; k < numLabels; k++)
  {
    unsigned int firstCount = 0;
    for (unsigned int j = 0; j < numLabels; j++)
      firstCount += countMatrix(k, j);

    unsigned int secondCount = 0;
    for (unsigned int j = 0; j < numLabels; j++)
      secondCount += countMatrix(j, k);

    sumEF += firstCount*secondCount;
  }
  sumEF /= numSamples;

  return (sumAgreements - sumEF) / (numSamples - sumEF);
}

#endif
