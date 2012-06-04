
// Jaccard similarity coefficient
// Input: binary images
// 2 * intersect(A, B) / union(A, B)

#ifndef _JaccardOverlapImageToImageMetric_h
#define _JaccardOverlapImageToImageMetric_h

#include "AbstractValidationMetric.h"

#include "itkImageToImageMetric.h"
#include "itkSmartPointer.h"

template <class TFixedImage, class TMovingImage>
class ITK_EXPORT JaccardOverlapImageToImageMetric :
  public itk::ImageToImageMetric<TFixedImage, TMovingImage>, public AbstractValidationMetric
{

public:

  // Validation metric properties
  bool IsInputBinary() { return true; }
  bool IsSymmetric() { return true; }
  double GetBestScore() { return 1.0; }
  double GetWorstScore() { return 0.0; }

  /** Standard class typedefs. */
  typedef JaccardOverlapImageToImageMetric           Self;
  typedef itk::ImageToImageMetric<TFixedImage, TMovingImage> Superclass;
  typedef itk::SmartPointer<Self>                            Pointer;
  typedef itk::SmartPointer<const Self>                      ConstPointer;

  typedef TFixedImage FixedImageType;
  typedef TMovingImage MovingImageType;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(JaccardOverlapImageToImageMetric,
    itk::ImageToImageMetric);

  typedef typename Superclass::MeasureType MeasureType;
  typedef typename Superclass::TransformParametersType TransformParametersType;
  typedef typename Superclass::DerivativeType DerivativeType;

  /**  Get the value for single valued optimizers. */
  MeasureType GetValue() const;

  MeasureType GetValue(const TransformParametersType& p) const
  { // TODO: apply transform with nearest neighbor interpolation
    return this->GetValue(); }

  void GetDerivative(const TransformParametersType& p, DerivativeType& dp) const
  { itkExceptionMacro(<< "Not implemented"); }

  void GetValueAndDerivative(const TransformParametersType& p, MeasureType& v, DerivativeType& dp) const
  { itkExceptionMacro(<< "Not implemented"); }

protected:

  JaccardOverlapImageToImageMetric();
  virtual ~JaccardOverlapImageToImageMetric();

};

#ifndef ITK_MANUAL_INSTANTIATION
#include "JaccardOverlapImageToImageMetric.txx"
#endif

#endif
