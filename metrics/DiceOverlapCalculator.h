
// Dice similarity coefficients
//
// 2 * intersect(A, B) / (|A| + |B|)
//
// Label image must be of unsigned type

#ifndef _DiceOverlapCalculator_h
#define _DiceOverlapCalculator_h

#include "DynArray.h"

template <class TLabelImage>
class DiceOverlapCalculator
{

public:

  typedef TLabelImage LabelImageType;
  typedef typename TLabelImage::Pointer LabelImagePointer;

  DiceOverlapCalculator();
  ~DiceOverlapCalculator();

  void SetFirstInput(TLabelImage* img);
  void SetSecondInput(TLabelImage* img);

  DynArray<double> ComputeOverlaps();

private:

  LabelImagePointer m_Input1;
  LabelImagePointer m_Input2;

};

#ifndef MU_MANUAL_INSTANTIATION
#include "DiceOverlapCalculator.txx"
#endif

#endif
