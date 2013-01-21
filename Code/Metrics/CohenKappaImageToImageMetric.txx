
#ifndef _CohenKappaImageToImageMetric_txx
#define _CohenKappaImageToImageMetric_txx

#include "CohenKappaImageToImageMetric.h"

#include "itkImageRegionConstIterator.h"

#include "vnl/vnl_matrix.h"

template <class TFixedImage, class TMovingImage>
CohenKappaImageToImageMetric<TFixedImage, TMovingImage>
::CohenKappaImageToImageMetric()
{
  m_IgnoreBackground = true;
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
  unsigned int maxLabel = 0;
  unsigned int numSamples = 0;

  fixedIt.GoToBegin();
  movingIt.GoToBegin();
  while (!fixedIt.IsAtEnd() && !movingIt.IsAtEnd())
  {
    unsigned int r = (unsigned int)fixedIt.Get();
    unsigned int c = (unsigned int)movingIt.Get();

    if (r > maxLabel)
        maxLabel = r;

    if (c > maxLabel)
      maxLabel = c;

    if (m_IgnoreBackground)
    {
      if (r != 0 || c != 0)
        numSamples++;
    }
    else
    {
      numSamples++;
    }

    ++fixedIt;
    ++movingIt;
  }

  unsigned int numClasses = maxLabel + 1;
  //unsigned int numClasses = maxLabel;
  //if (!m_IgnoreBackground)
  //  numClasses++;

  // Fill in matrix that counts agreements/disagreements
  vnl_matrix<unsigned int> countMatrix(numClasses, numClasses, 0);

  fixedIt.GoToBegin();
  movingIt.GoToBegin();
  while (!fixedIt.IsAtEnd() && !movingIt.IsAtEnd())
  {
    unsigned int r = (unsigned int)fixedIt.Get();
    unsigned int c = (unsigned int)movingIt.Get();

    if (m_IgnoreBackground)
    {
      if (r != 0 || c != 0)
      {
        //countMatrix(r-1, c-1) += 1;
        countMatrix(r, c) += 1;
      }
    }
    else
    {
      countMatrix(r, c) += 1;
    }

    ++fixedIt;
    ++movingIt;
  }

  // Process the matrix
  unsigned int sumAgreements = 0;
  for (unsigned int k = 0; k < numClasses; k++)
    sumAgreements += countMatrix(k, k);

  double sumEF = 0;
  if (m_IgnoreBackground)
  {
    for (unsigned int k = 1; k < numClasses; k++)
    {
      unsigned int firstCount = 0;
      for (unsigned int j = 1; j < numClasses; j++)
        firstCount += countMatrix(k, j);

      unsigned int secondCount = 0;
      for (unsigned int j = 1; j < numClasses; j++)
        secondCount += countMatrix(j, k);

      sumEF += firstCount*secondCount;
    }
  }
  else
  {
    for (unsigned int k = 0; k < numClasses; k++)
    {
      unsigned int firstCount = 0;
      for (unsigned int j = 0; j < numClasses; j++)
        firstCount += countMatrix(k, j);

      unsigned int secondCount = 0;
      for (unsigned int j = 0; j < numClasses; j++)
        secondCount += countMatrix(j, k);

      sumEF += firstCount*secondCount;
    }
  }
  sumEF /= numSamples;

std::cout << "numSamples = " << numSamples << std::endl;
std::cout << "sumAgreements = " << sumAgreements << std::endl;
std::cout << "sumEF = " << sumEF << std::endl;

  return (sumAgreements - sumEF) / (numSamples - sumEF);
}

#endif
