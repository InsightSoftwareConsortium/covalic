
// Checks that two images are comparable in origin, spacing, direction.
// Input: binary images, where fixed image denotes truth

#ifndef _ImageToImageValidator_h
#define _ImageToImageValidator_h

#include "AbstractValidationMetric.h"

#include "itkImageToImageMetric.h"
#include "itkSmartPointer.h"

template <class TFixedImage, class TMovingImage>
class ITK_EXPORT ImageToImageValidator :
  public itk::ImageToImageMetric<TFixedImage, TMovingImage>, public AbstractValidationMetric
{

public:

  // Validation metric properties
  bool IsInputBinary() { return true; }
  bool IsSymmetric() { return false; }
  double GetBestScore() { return 0.0; }
  double GetWorstScore() { return 1.0; }

  /** Standard class typedefs. */
  typedef ImageToImageValidator           Self;
  typedef itk::ImageToImageMetric<TFixedImage, TMovingImage> Superclass;
  typedef itk::SmartPointer<Self>                            Pointer;
  typedef itk::SmartPointer<const Self>                      ConstPointer;

  typedef TFixedImage FixedImageType;
  typedef TMovingImage MovingImageType;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(ImageToImageValidator,
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

  ImageToImageValidator();
  virtual ~ImageToImageValidator();


};

#ifndef ITK_MANUAL_INSTANTIATION
#include "ImageToImageValidator.txx"
#endif

#endif
