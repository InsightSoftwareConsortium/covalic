
// Bhattacharyya  metric for two probability density (vector) images
// Vectors in each pixel must sum to one and each element in [0, 1]

#ifndef _BhattacharyyaImageToImageMetric_h
#define _BhattacharyyaImageToImageMetric_h

#include "itkImageToImageMetric.h"
#include "itkSmartPointer.h"

template <class TFixedImage, class TMovingImage>
class ITK_EXPORT BhattacharyyaImageToImageMetric :
  public itk::ImageToImageMetric<TFixedImage, TMovingImage>
{

public:

  /** Standard class typedefs. */
  typedef BhattacharyyaImageToImageMetric           Self;
  typedef itk::ImageToImageMetric<TFixedImage, TMovingImage> Superclass;
  typedef itk::SmartPointer<Self>                            Pointer;
  typedef itk::SmartPointer<const Self>                      ConstPointer;

  typedef TFixedImage FixedImageType;
  typedef TMovingImage MovingImageType;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(BhattacharyyaImageToImageMetric,
    itk::ImageToImageMetric);

  typedef typename Superclass::MeasureType MeasureType;

  void SetEpsilon(MeasureType e) { m_Epsilon = e; }
  MeasureType GetEpsilon() const { return m_Epsilon; }

  /**  Get the value for single valued optimizers. */
  MeasureType GetValue() const;

  MeasureType GetValue(const itk::Array<double>& p) const
  { // TODO: apply transform with nearest neighbor interpolation
    return this->GetValue(); }

  MeasureType GetDerivative(const itk::Array<double>& p) const
  { itkExceptionMacro(<< "Not implemented"); return 0; }
  virtual void GetDerivative(const itk::Array<double>& p, itk::Array<double>& q) const
  { itkExceptionMacro(<< "Not implemented"); }

protected:

  BhattacharyyaImageToImageMetric();
  virtual ~BhattacharyyaImageToImageMetric();

  MeasureType m_Epsilon;

// TODO: transform member, default to identity

};

#ifndef ITK_MANUAL_INSTANTIATION
#include "BhattacharyyaImageToImageMetric.txx"
#endif

#endif
