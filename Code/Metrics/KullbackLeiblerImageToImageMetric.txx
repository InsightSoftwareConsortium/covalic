
#ifndef _KullbackLeiblerImageToImageMetric_txx

#include "KullbackLeiblerImageToImageMetric.h"

#include "itkImageRegionIterator.h"

#include <cmath>

template <class TFixedImage, class TMovingImage>
KullbackLeiblerImageToImageMetric<TFixedImage, TMovingImage>
::KullbackLeiblerImageToImageMetric()
{
  m_Epsilon = 1e-8;
}

template <class TFixedImage, class TMovingImage>
KullbackLeiblerImageToImageMetric<TFixedImage, TMovingImage>
::~KullbackLeiblerImageToImageMetric()
{

}

template <class TFixedImage, class TMovingImage>
typename KullbackLeiblerImageToImageMetric<TFixedImage,TMovingImage>::MeasureType
KullbackLeiblerImageToImageMetric<TFixedImage, TMovingImage>
::GetValue() const
{
  if (Superclass::m_FixedImage.IsNull() || Superclass::m_MovingImage.IsNull())
    itkExceptionMacro(<< "Need two input classification images");

  // Define iterators
  typedef itk::ImageRegionConstIterator<FixedImageType> FixedIteratorType;
  typedef itk::ImageRegionConstIterator<MovingImageType> MovingIteratorType;

  FixedIteratorType fixedIt(Superclass::m_FixedImage, Superclass::m_FixedImage->GetRequestedRegion());
  MovingIteratorType movingIt(Superclass::m_MovingImage, Superclass::m_MovingImage->GetRequestedRegion());

  // TODO:
  // Check to make sure inputs are pdfs (each voxel sums to one, in (0, 1]

  typename FixedImageType::VectorLengthType numComponents =
    Superclass::m_FixedImage->GetVectorLength();

  // Get the Kullback-Leibler overlap for the two pdf images
  MeasureType klMetric = 0;

  fixedIt.GoToBegin();
  movingIt.GoToBegin();
  while (!fixedIt.IsAtEnd() && !movingIt.IsAtEnd())
  {
    FixedImagePixelType pa = fixedIt.Get();
    FixedImagePixelType pb = movingIt.Get();

    MeasureType sum_pa = 0; 
    MeasureType sum_pb = 0;

    for (unsigned int c = 0; c < numComponents; c++)
    {
      double pa_c = pa[c];
      double pb_c = pb[c];

      if (pa_c < m_Epsilon)
        pa_c = m_Epsilon;
      if (pb_c < m_Epsilon)
        pb_c = m_Epsilon;

      sum_pa += pa_c;
      sum_pb += pb_c;

      klMetric += pa_c * (log(pa_c) - log(pb_c));
    }

    if (abs(sum_pa - 1.0) > m_Epsilon)
      itkExceptionMacro(<< "Fixed image is not a probability density image");
    if (abs(sum_pb - 1.0) > m_Epsilon)
      itkExceptionMacro(<< "Moving image is not a probability density image");
  
    ++fixedIt;
    ++movingIt;
  }

  return klMetric;
}

#endif
