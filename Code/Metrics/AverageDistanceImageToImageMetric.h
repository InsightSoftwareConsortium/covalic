
// Average boundary distances, given two binary image masks
//
// Both fixed and moving images should have the same unsigned type
// Only handles 3D images

#ifndef _AverageDistanceImageToImageMetric_h
#define _AverageDistanceImageToImageMetric_h

#include "AbstractValidationMetric.h"

#include "itkImageToImageMetric.h"

#include "vnl/vnl_math.h"

template <class TFixedImage, class TMovingImage>
class AverageDistanceImageToImageMetric :
  public itk::ImageToImageMetric<TFixedImage, TMovingImage>, public AbstractValidationMetric
{

public:

  // Validation metric properties
  bool IsInputBinary() { return true; }
  bool IsSymmetric() { return true; }
  double GetBestScore() { return 0; }
  double GetWorstScore() { return vnl_huge_val(1.0); }

  /** Standard class typedefs. */
  typedef AverageDistanceImageToImageMetric           Self;
  typedef itk::ImageToImageMetric<TFixedImage, TMovingImage> Superclass;
  typedef itk::SmartPointer<Self>                            Pointer;
  typedef itk::SmartPointer<const Self>                      ConstPointer;

  typedef TFixedImage FixedImageType;
  typedef TMovingImage MovingImageType;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(AverageDistanceImageToImageMetric,
    itk::ImageToImageMetric);

  typedef typename Superclass::MeasureType MeasureType;
  typedef typename Superclass::TransformParametersType TransformParametersType;
  typedef typename Superclass::DerivativeType DerivativeType;

  typedef typename TFixedImage::Pointer FixedImagePointer;
  typedef typename TFixedImage::IndexType FixedImageIndexType;
  typedef typename TFixedImage::PixelType FixedImagePixelType;
  typedef typename TFixedImage::PointType FixedImagePointType;
  typedef typename TFixedImage::SizeType FixedImageSizeType;
  typedef typename TFixedImage::SpacingType FixedImageSpacingType;

  MeasureType GetValue() const;

  MeasureType GetValue(const TransformParametersType& p) const
  { // TODO: apply transform with nearest neighbor interpolation
    return this->GetValue(); }

  void GetDerivative(const TransformParametersType& p, DerivativeType& dp) const
  { itkExceptionMacro(<< "Not implemented"); }

  void GetValueAndDerivative(const TransformParametersType& p, MeasureType& v, DerivativeType& dp) const
  { itkExceptionMacro(<< "Not implemented"); }

  void BlurringOn() { m_DoBlurring = true; }
  void BlurringOff() { m_DoBlurring = false; }

protected:

  AverageDistanceImageToImageMetric();
  ~AverageDistanceImageToImageMetric();

  double ComputeNonSymmetricDistance(const FixedImageType*, const MovingImageType*) const;

private:

  bool m_DoBlurring;

};

#ifndef MU_MANUAL_INSTANTIATION
#include "AverageDistanceImageToImageMetric.txx"
#endif

#endif
