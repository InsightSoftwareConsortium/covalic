
// Kullback-Leibler metric for two probability density (vector) images
// Vectors in each pixel must sum to one and each element in (0, 1]
// 
// Fixed image is taken to be the reference ground truth
//
// Reference:
// Peter Lorenzen, Marcel Prastawa, Brad Davis, Guido Gerig, Elizabeth Bullitt,
// and Sarang Joshi, Multi-Modal Image Set Registration and Atlas Formation,
// Medical Image Analysis MEDIA, Elsevier, Vol 10 issue 3, pp 440-451, May 2006

#ifndef _KullbackLeiblerImageToImageMetric_h
#define _KullbackLeiblerImageToImageMetric_h

#include "itkImageToImageMetric.h"
#include "itkSmartPointer.h"

template <class TFixedImage, class TMovingImage>
class ITK_EXPORT KullbackLeiblerImageToImageMetric :
  public itk::ImageToImageMetric<TFixedImage, TMovingImage>
{

public:

  /** Standard class typedefs. */
  typedef KullbackLeiblerImageToImageMetric           Self;
  typedef itk::ImageToImageMetric<TFixedImage, TMovingImage> Superclass;
  typedef itk::SmartPointer<Self>                            Pointer;
  typedef itk::SmartPointer<const Self>                      ConstPointer;

  typedef TFixedImage FixedImageType;
  typedef TMovingImage MovingImageType;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(KullbackLeiblerImageToImageMetric,
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

  KullbackLeiblerImageToImageMetric();
  virtual ~KullbackLeiblerImageToImageMetric();

  MeasureType m_Epsilon;

// TODO: transform member, default to identity

};

#ifndef ITK_MANUAL_INSTANTIATION
#include "KullbackLeiblerImageToImageMetric.txx"
#endif

#endif
