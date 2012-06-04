
// Cohen's kappa coefficient for measuring agreement between two observers
//
// Input: two label images
//
// Label image type must be of unsigned type
//

#ifndef _CohenKappaImageToImageMetric_h
#define _CohenKappaImageToImageMetric_h

#include "AbstractValidationMetric.h"

#include "itkImageToImageMetric.h"

template <class TFixedImage, class TMovingImage>
class CohenKappaImageToImageMetric :
  public itk::ImageToImageMetric<TFixedImage, TMovingImage>, public AbstractValidationMetric
{

public:

 // Validation metric properties
  bool IsInputBinary() { return false; }
  bool IsSymmetric() { return true; }
  double GetBestScore() { return 1.0; }
  double GetWorstScore() { return 0.0; }

  /** Standard class typedefs. */
  typedef CohenKappaImageToImageMetric           Self;
  typedef itk::ImageToImageMetric<TFixedImage, TMovingImage> Superclass;
  typedef itk::SmartPointer<Self>                            Pointer;
  typedef itk::SmartPointer<const Self>                      ConstPointer;

  typedef TFixedImage FixedImageType;
  typedef TMovingImage MovingImageType;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(CohenKappaImageToImageMetric,
    itk::ImageToImageMetric);

  typedef typename Superclass::MeasureType MeasureType;
  typedef typename Superclass::TransformParametersType TransformParametersType;
  typedef typename Superclass::DerivativeType DerivativeType;

  MeasureType GetValue() const;

  MeasureType GetValue(const TransformParametersType& p) const
  { // TODO: apply transform with nearest neighbor interpolation
    return this->GetValue(); }

  void GetDerivative(const TransformParametersType& p, DerivativeType& dp) const
  { itkExceptionMacro(<< "Not implemented"); }

  void GetValueAndDerivative(const TransformParametersType& p, MeasureType& v, DerivativeType& dp) const
  { itkExceptionMacro(<< "Not implemented"); }

protected:

  CohenKappaImageToImageMetric();
  virtual ~CohenKappaImageToImageMetric();

};

#ifndef ITK_MANUAL_INSTANTIATION
#include "CohenKappaImageToImageMetric.txx"
#endif

#endif
