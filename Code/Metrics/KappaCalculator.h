
//
// Kappa coeff for agreement between two observers
//
// Label image type must be of unsigned type
//

#ifndef _KappaCalculator_h
#define _KappaCalculator_h

template <class TLabelImage>
class KappaCalculator
{

public:

  typedef TLabelImage LabelImageType;
  typedef typename TLabelImage::Pointer LabelImagePointer;

  KappaCalculator();
  ~KappaCalculator();

  void SetFirstInput(TLabelImage* img);
  void SetSecondInput(TLabelImage* img);

  double ComputeKappa();

private:

  LabelImagePointer m_Input1;
  LabelImagePointer m_Input2;

};

#ifndef ITK_MANUAL_INSTANTIATION
#include "KappaCalculator.txx"
#endif

#endif
