
#ifndef _KappaCalculator_txx

#include "itkImageRegionIterator.h"

#include "vnl/vnl_matrix.h"

#include "KappaCalculator.h"

#include "muException.h"

template <class TLabelImage>
KappaCalculator<TLabelImage>
::KappaCalculator()
{

}

template <class TLabelImage>
KappaCalculator<TLabelImage>
::~KappaCalculator()
{

}

template <class TLabelImage>
void
KappaCalculator<TLabelImage>
::SetFirstInput(TLabelImage* img)
{
  m_Input1 = img;
}

template <class TLabelImage>
void
KappaCalculator<TLabelImage>
::SetSecondInput(TLabelImage* img)
{
  m_Input2 = img;
}

template <class TLabelImage>
double
KappaCalculator<TLabelImage>
::ComputeKappa()
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

  // Fill in matrix that counts agreements/disagreements
  vnl_matrix<unsigned int> countMatrix(numLabels, numLabels);

  it1.GoToBegin();
  it2.GoToBegin();
  while (!it1.IsAtEnd())
  {
    unsigned int r = (unsigned int)it1.Get();
    unsigned int c = (unsigned int)it2.Get();

    if (r != 0 && c != 0)
    {
      countMatrix(r-1, c-1) += 1;
    }

    ++it1;
    ++it2;
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
