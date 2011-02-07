
// Dice similarity coefficients
//
// 2 * intersect(A, B) / (|A| + |B|)
//
// Label image must be of unsigned type

#ifndef _DiceOverlapImageToImageMetric_h
#define _DiceOverlapImageToImageMetric_h


template <class TFixedImage, class TMovingImage>
class ITK_EXPORT DiceOverlapImageToImageMetric :
public itk::ImageToImageMetric<TFixedImage, TMovingImage>
{

public:

  typedef TLabelImage LabelImageType;
  /** Standard class typedefs. */
  typedef DiceOverlapImageToImageMetric           Self;
  typedef itk::ImageToImageMetric<TFixedImage, TMovingImage> Superclass;
  typedef SmartPointer<Self>                                     Pointer;
  typedef SmartPointer<const Self>                               ConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(DiceOverlapImageToImageMetric,
    itk::ImageToImageMetric);

  typedef typename Superclass::MeasureType MeasureType;

  /**  Get the value for single valued optimizers. */
  MeasureType GetValue(const TransformParametersType& parameters) const;

  // TODO
  // For label image?
  // std::vector<double> GetValues();

protected:

  DiceOverlapImageToImageMetric();
  virtual ~DiceOverlapImageToImageMetric();

};

#ifndef ITK_MANUAL_INSTANTIATION
#include "DiceOverlapImageToImageMetric.txx"
#endif

#endif
