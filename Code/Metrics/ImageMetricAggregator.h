
#ifndef _ImageMetricAggregator_h
#define _ImageMetricAggregator_h

#include "itkObject.h"
#include "itkImageToImageMetric.h"

#include <vector>

template <class TImage>
class ITK_EXPORT ImageMetricAggregator<TImage>: public itk::Object
{
public:

  typedef TImage ImageType;
  typedef typename TImage::Pointer ImagePointer;

  typedef itkImageToImageMetric<TImage,TImage> MetricType;
  typedef typename MetricType::Pointer MetricPointer;

  void SetFixedImage(const ImageType* img)
  { m_FixedImage = img; }
  void SetMovingImage(const ImageType* img)
  { m_MovingImage = img; }

  void AddMetricObject(const MetricType* obj)
  { m_MetricObjects.push_back(obj); }

  virtual double GetAggregateScore() = 0;

protected:

  ImagePointer m_FixedImage;
  ImagePointer m_MovingImage;

  std::vector<MetricPointer> m_MetricObjects;

};

#endif
