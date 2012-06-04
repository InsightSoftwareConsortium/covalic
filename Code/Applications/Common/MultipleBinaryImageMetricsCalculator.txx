
#ifndef _MultipleBinaryImageMetricsCalculator_txx
#define _MultipleBinaryImageMetricsCalculator_txx

#include "MultipleBinaryImageMetricsCalculator.h"

#include "itkBinaryThresholdImageFilter.h"
#include "itkMinimumMaximumImageCalculator.h"

#include "itkImageRegionIterator.h"

template <class TFixedImage, class TMovingImage, class TMetric>
MultipleBinaryImageMetricsCalculator<TFixedImage, TMovingImage, TMetric>
::MultipleBinaryImageMetricsCalculator()
{

}

template <class TFixedImage, class TMovingImage, class TMetric>
MultipleBinaryImageMetricsCalculator<TFixedImage, TMovingImage, TMetric>
::~MultipleBinaryImageMetricsCalculator()
{

}

template <class TFixedImage, class TMovingImage, class TMetric>
void
MultipleBinaryImageMetricsCalculator<TFixedImage, TMovingImage, TMetric>
::SetFixedImage(FixedImageType* img)
{
  m_FixedImage = img;
}

template <class TFixedImage, class TMovingImage, class TMetric>
void
MultipleBinaryImageMetricsCalculator<TFixedImage, TMovingImage, TMetric>
::SetMovingImage(MovingImageType* img)
{
  m_MovingImage = img;
}

template <class TFixedImage, class TMovingImage, class TMetric>
unsigned int
MultipleBinaryImageMetricsCalculator<TFixedImage, TMovingImage, TMetric>
::GetNumberOfValues() const
{
  return m_MetricValues.size();
}

template <class TFixedImage, class TMovingImage, class TMetric>
double
MultipleBinaryImageMetricsCalculator<TFixedImage, TMovingImage, TMetric>
::GetValue(unsigned int i)
{
  return m_MetricValues[i];
}

template <class TFixedImage, class TMovingImage, class TMetric>
void
MultipleBinaryImageMetricsCalculator<TFixedImage, TMovingImage, TMetric>
::Update()
{
  if (m_FixedImage.IsNull())
    itkExceptionMacro(<< "Fixed image undefined");

  if (m_MovingImage.IsNull())
    itkExceptionMacro(<< "Moving image undefined");

  typedef itk::MinimumMaximumImageCalculator<FixedImageType> FixedMinMaxCalculator;
  typename FixedMinMaxCalculator::Pointer fminmax = FixedMinMaxCalculator::New();
  fminmax->SetImage(m_FixedImage);
  fminmax->ComputeMaximum();

  unsigned int maxLabel = fminmax->GetMaximum();

  typedef itk::MinimumMaximumImageCalculator<MovingImageType> MovingMinMaxCalculator;
  typename MovingMinMaxCalculator::Pointer mminmax = MovingMinMaxCalculator::New();
  mminmax->SetImage(m_MovingImage);
  mminmax->ComputeMaximum();

  if (mminmax->GetMaximum() > maxLabel)
    maxLabel = mminmax->GetMaximum();

  m_MetricValues.clear();

  for (unsigned int label = 1; label <= maxLabel; label++)
  {
    itkDebugMacro(<< "Computing metric for label " << label << "\n");

    typedef itk::BinaryThresholdImageFilter<FixedImageType, FixedImageType>
      FixedThresholderType;
    typename FixedThresholderType::Pointer thresf = FixedThresholderType::New();
    thresf->SetInput(m_FixedImage);
    thresf->SetLowerThreshold(label);
    thresf->SetUpperThreshold(label);
    thresf->Update();

    typedef itk::BinaryThresholdImageFilter<MovingImageType, MovingImageType>
      MovingThresholderType;
    typename MovingThresholderType::Pointer thresm = MovingThresholderType::New();
    thresm->SetInput(m_MovingImage);
    thresm->SetLowerThreshold(label);
    thresm->SetUpperThreshold(label);
    thresm->Update();

    typename MetricType::Pointer metric = MetricType::New();
    metric->SetFixedImage(thresf->GetOutput());
    metric->SetMovingImage(thresm->GetOutput());

    m_MetricValues.push_back(metric->GetValue());
  }


}


#endif
