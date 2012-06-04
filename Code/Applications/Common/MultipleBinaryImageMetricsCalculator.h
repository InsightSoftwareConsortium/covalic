
#ifndef _MultipleBinaryImageMetricsCalculator_h
#define _MultipleBinaryImageMetricsCalculator_h

#include "itkObject.h"

#include <vector>

template <class TFixedImage, class TMovingImage, class TMetric>
class MultipleBinaryImageMetricsCalculator:
  public itk::Object
{

public:

  /** Standard class typedefs. */
  typedef MultipleBinaryImageMetricsCalculator               Self;
  typedef itk::Object                                        Superclass;
  typedef itk::SmartPointer<Self>                            Pointer;
  typedef itk::SmartPointer<const Self>                      ConstPointer;

  typedef TFixedImage FixedImageType;
  typedef TMovingImage MovingImageType;

  typedef typename FixedImageType::Pointer FixedImagePointer;
  typedef typename MovingImageType::Pointer MovingImagePointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  //typedef TMetric<TFixedImage, TMovingImage> MetricType;
  typedef TMetric MetricType;

  void SetFixedImage(FixedImageType* img);
  void SetMovingImage(MovingImageType* img);

  void Update();

  unsigned int GetNumberOfValues() const;

  double GetValue(unsigned int i);

protected:

  MultipleBinaryImageMetricsCalculator();
  ~MultipleBinaryImageMetricsCalculator();

  FixedImagePointer m_FixedImage;
  MovingImagePointer m_MovingImage;

  std::vector<double> m_MetricValues;
  

};

#ifndef ITK_MANUAL_INSTANTIATION
#include "MultipleBinaryImageMetricsCalculator.txx"
#endif


#endif
