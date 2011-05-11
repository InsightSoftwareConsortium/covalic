
// Cohen's kappa coefficient for measuring agreement between two observers
//
// Input: two label images
//
// Label image type must be of unsigned type
//

#ifndef _CohenKappaImageToImageMetric_h
#define _CohenKappaImageToImageMetric_h

#include "itkImageToImageMetric.h"

template <class TFixedImage, class TMovingImage>
class CohenKappaImageToImageMetric :
  public itk::ImageToImageMetric<TFixedImage, TMovingImage>
{

public:

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

  MeasureType GetValue() const;

  MeasureType GetValue(const itk::Array<double>& p) const
  { // TODO: apply transform with nearest neighbor interpolation
    return this->GetValue(); }

  MeasureType GetDerivative(const itk::Array<double>& p) const
  { itkExceptionMacro(<< "Not implemented"); return 0; }
  virtual void GetDerivative(const itk::Array<double>& p, itk::Array<double>& q) const
  { itkExceptionMacro(<< "Not implemented"); }

protected:

  CohenKappaImageToImageMetric();
  virtual ~CohenKappaImageToImageMetric();

};

#ifndef ITK_MANUAL_INSTANTIATION
#include "CohenKappaImageToImageMetric.txx"
#endif

#endif
