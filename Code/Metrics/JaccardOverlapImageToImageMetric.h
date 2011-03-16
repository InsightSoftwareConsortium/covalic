
// Jaccard similarity coefficient
// Input: binary images
// 2 * intersect(A, B) / union(A, B)

#ifndef _JaccardOverlapImageToImageMetric_h
#define _JaccardOverlapImageToImageMetric_h

#include "itkImageToImageMetric.h"
#include "itkSmartPointer.h"

template <class TFixedImage, class TMovingImage>
class ITK_EXPORT JaccardOverlapImageToImageMetric :
  public itk::ImageToImageMetric<TFixedImage, TMovingImage>
{

public:

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

  /**  Get the value for single valued optimizers. */
  MeasureType GetValue() const;

  MeasureType GetValue(const itk::Array<double>& p) const
  { // TODO: apply transform with nearest neighbor interpolation
    return this->GetValue(); }

  MeasureType GetDerivative(const itk::Array<double>& p) const
  { return 0; }
  virtual void GetDerivative(const itk::Array<double>& p, itk::Array<double>& q) const
  { return; }

  // TODO
  // For label image?
  // std::vector<double> GetValues();

protected:

  JaccardOverlapImageToImageMetric();
  virtual ~JaccardOverlapImageToImageMetric();

};

#ifndef ITK_MANUAL_INSTANTIATION
#include "JaccardOverlapImageToImageMetric.txx"
#endif

#endif
