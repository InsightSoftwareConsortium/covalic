
// Cohen's kappa coefficient for measuring agreement between two observers
//
// Input: two label images
//
// Label image type must be of unsigned type
//

#ifndef _CohenKappaImageToImageMetric_h
#define _CohenKappaImageToImageMetric_h

#include "itkImageToImageMetric.h"

template <class TLabelImage>
class CohenKappaImageToImageMetric: public itk::ImageToImageMetric<TFixedImage, TMovingImage>
{

public:

  typedef TLabelImage LabelImageType;
  typedef typename TLabelImage::Pointer LabelImagePointer;

  CohenKappaImageToImageMetric();
  ~CohenKappaImageToImageMetric();

  MeasureType GetValue() const;

protected:

};

#ifndef ITK_MANUAL_INSTANTIATION
#include "CohenKappaImageToImageMetric.txx"
#endif

#endif
