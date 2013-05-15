
#ifndef _ImageToImageValidator_txx
#define _ImageToImageValidator_txx

#include "ImageToImageValidator.h"

#include "itkImageRegionIterator.h"

#include "itkPoint.h"

template <class TFixedImage, class TMovingImage>
ImageToImageValidator<TFixedImage, TMovingImage>
::ImageToImageValidator()
{

}

template <class TFixedImage, class TMovingImage>
ImageToImageValidator<TFixedImage, TMovingImage>
::~ImageToImageValidator()
{

}

template <class TFixedImage, class TMovingImage>
typename ImageToImageValidator<TFixedImage,TMovingImage>::MeasureType
ImageToImageValidator<TFixedImage, TMovingImage>
::GetValue() const
{
  if (Superclass::m_FixedImage.IsNull() || Superclass::m_MovingImage.IsNull())
    itkExceptionMacro(<< "Need two input classification images");

/*
  std::cout << "Truth Origin" <<  std::endl;
  std::cout << Superclass::m_FixedImage->GetOrigin() << std::endl;
  std::cout << "Result Origin" <<  std::endl;
  std::cout << Superclass::m_MovingImage->GetOrigin() << std::endl;
  std::cout << std::endl;

  std::cout << "Truth Spacing" <<  std::endl;
  std::cout << Superclass::m_FixedImage->GetSpacing() << std::endl;
  std::cout << "Result Spacing" <<  std::endl;
  std::cout << Superclass::m_MovingImage->GetSpacing() << std::endl;
  std::cout << std::endl;

  std::cout << "Truth Direction" <<  std::endl;
  std::cout << Superclass::m_FixedImage->GetDirection() << std::endl;
  std::cout << "Result Direction" <<  std::endl;
  std::cout << Superclass::m_MovingImage->GetDirection() << std::endl;
  std::cout << std::endl;

  std::cout << "Truth LargestPossibleRegionSize" <<  std::endl;
  std::cout << Superclass::m_FixedImage->GetLargestPossibleRegion().GetSize() << std::endl;
  std::cout << "Result LargestPossibleRegionSize" <<  std::endl;
  std::cout << Superclass::m_MovingImage->GetLargestPossibleRegion().GetSize() << std::endl;
  std::cout << std::endl;
*/

  if (Superclass::m_FixedImage->GetOrigin() != Superclass::m_MovingImage->GetOrigin())
    return -1.0;
  if (Superclass::m_FixedImage->GetSpacing() != Superclass::m_MovingImage->GetSpacing())
    return -1.0;
  if (Superclass::m_FixedImage->GetDirection() != Superclass::m_MovingImage->GetDirection())
    return -1.0;
  if (Superclass::m_FixedImage->GetLargestPossibleRegion().GetSize()
      != Superclass::m_MovingImage->GetLargestPossibleRegion().GetSize())
    return -1.0;

  return 0;
}

#endif
