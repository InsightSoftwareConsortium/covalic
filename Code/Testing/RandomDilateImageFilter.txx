
#ifndef _RandomDilateImageFilter_txx
#define _RandomDilateImageFilter_txx

#include "RandomDilateImageFilter.h"

#include "itkBinaryBallStructuringElement.h"
#include "itkBinaryDilateImageFilter.h"
#include "itkBinaryErodeImageFilter.h"

#include "itkConstNeighborhoodIterator.h"
#include "itkNeighborhoodInnerProduct.h"
#include "itkImageRegionIterator.h"
#include "itkImageRegionIteratorWithIndex.h"
#include "itkNeighborhoodAlgorithm.h"
#include "itkZeroFluxNeumannBoundaryCondition.h"
#include "itkOffset.h"
#include "itkProgressReporter.h"

#include <cstdlib>
#include <sstream>

template <class TInputImage, class TOutputImage>
RandomDilateImageFilter<TInputImage, TOutputImage>
::RandomDilateImageFilter()
{
  m_Radius.Fill(1);
}

template <class TInputImage, class TOutputImage>
void
RandomDilateImageFilter<TInputImage, TOutputImage>
::SetRadius(long n)
{
  m_Radius.Fill(n);
}

template <class TInputImage, class TOutputImage>
void 
RandomDilateImageFilter<TInputImage, TOutputImage>
::GenerateInputRequestedRegion() throw (itk::InvalidRequestedRegionError)
{
  // call the superclass' implementation of this method
  Superclass::GenerateInputRequestedRegion();
  
  // get pointers to the input and output
  typename Superclass::InputImagePointer inputPtr = 
    const_cast< TInputImage * >( this->GetInput() );
  typename Superclass::OutputImagePointer outputPtr = this->GetOutput();

  InputSizeType radius;
  radius.Fill(1);
  
  if ( !inputPtr || !outputPtr )
    {
    return;
    }

  // get a copy of the input requested region (should equal the output
  // requested region)
  typename TInputImage::RegionType inputRequestedRegion;
  inputRequestedRegion = inputPtr->GetRequestedRegion();

  // pad the input requested region by the operator radius
  inputRequestedRegion.PadByRadius( radius );

  // crop the input requested region at the input's largest possible region
  if ( inputRequestedRegion.Crop(inputPtr->GetLargestPossibleRegion()) )
    {
    inputPtr->SetRequestedRegion( inputRequestedRegion );
    return;
    }
  else
    {
    // Couldn't crop the region (requested region is outside the largest
    // possible region).  Throw an exception.

    // store what we tried to request (prior to trying to crop)
    inputPtr->SetRequestedRegion( inputRequestedRegion );
    
    // build an exception
    itk::InvalidRequestedRegionError e(__FILE__, __LINE__);
    //itk::OStringStream msg;
    std::ostringstream msg;
    msg << static_cast<const char *>(this->GetNameOfClass())
        << "::GenerateInputRequestedRegion()";
    e.SetLocation(msg.str().c_str());
    e.SetDescription("Requested region is (at least partially) outside the largest possible region.");
    e.SetDataObject(inputPtr);
    throw e;
    }
}


template< class TInputImage, class TOutputImage>
void
RandomDilateImageFilter< TInputImage, TOutputImage>
::ThreadedGenerateData(const OutputImageRegionType& outputRegionForThread,
                       int threadId)
{
  // Detect border pixels, select at random

  InputSizeType radius;
  radius.Fill(1);

  unsigned int i;
  itk::ZeroFluxNeumannBoundaryCondition<InputImageType> nbc;

  itk::ConstNeighborhoodIterator<InputImageType> bit;
  itk::ImageRegionIterator<OutputImageType> it;
  
  // Allocate output
  typename OutputImageType::Pointer output = this->GetOutput();
  typename InputImageType::ConstPointer input  = this->GetInput();
  
  // Find the data-set boundary "faces"
  typename itk::NeighborhoodAlgorithm::ImageBoundaryFacesCalculator<InputImageType>::FaceListType faceList;
  itk::NeighborhoodAlgorithm::ImageBoundaryFacesCalculator<InputImageType> bC;
  faceList = bC(input, outputRegionForThread, radius);

  typename itk::NeighborhoodAlgorithm::ImageBoundaryFacesCalculator<InputImageType>::FaceListType::iterator fit;

  // support progress methods/callbacks
  itk::ProgressReporter progress(this, threadId, outputRegionForThread.GetNumberOfPixels());
  
  // Process each of the boundary faces.  These are N-d regions which border
  // the edge of the buffer.
  for (fit=faceList.begin(); fit != faceList.end(); ++fit)
    { 
    bit = itk::ConstNeighborhoodIterator<InputImageType>(radius,
                                                    input, *fit);
    unsigned int neighborhoodSize = bit.Size();
    it = itk::ImageRegionIterator<OutputImageType>(output, *fit);

    bit.OverrideBoundaryCondition(&nbc);
    bit.GoToBegin();

    while ( ! bit.IsAtEnd() )
    {
      bool centerPix = bit.GetPixel(neighborhoodSize / 2) != 0;

      bool isborder = false;

      for (i = 0; i < neighborhoodSize; ++i)
      {
        bool neighPix = bit.GetPixel(i) != 0;
        if (centerPix != neighPix)
        {
          isborder = true;
          break;
        }
      }

      if (isborder)
      {
        double u = rand() / (double)RAND_MAX;

        if (u > 0.05)
          it.Set(0);
        else
          it.Set(1);
      }
      
      ++bit;
      ++it;
      progress.CompletedPixel();
      }
    }
}

template <class TInputImage, class TOutputImage>
void 
RandomDilateImageFilter<TInputImage, TOutputImage>
::AfterThreadedGenerateData()
{
  typedef itk::BinaryBallStructuringElement<OutputPixelType, OutputImageType::ImageDimension> StructElementType;
  typedef
    itk::BinaryDilateImageFilter<OutputImageType, OutputImageType,
      StructElementType> DilateType;
  typedef
    itk::BinaryErodeImageFilter<OutputImageType, OutputImageType,
      StructElementType> ErodeType;

  unsigned long maxRadius = m_Radius[0];
  for (unsigned int d = 0; d < OutputImageType::ImageDimension; d++)
  {
    if (maxRadius < m_Radius[d])
      maxRadius = m_Radius[d];
  }

  StructElementType structel;
  structel.SetRadius(maxRadius);
  structel.CreateStructuringElement();

  typename DilateType::Pointer dil = DilateType::New();
  dil->SetDilateValue(1);
  dil->SetInput(this->GetOutput());
  dil->SetKernel(structel);

  dil->Update();

/*
  typename ErodeType::Pointer erode = ErodeType::New();
  erode->SetErodeValue(1);
  erode->SetInput(dil->GetOutput());
  erode->SetKernel(structel);

  erode->Update();

  //this->GraftOutput(erode->GetOutput());
*/

  // Insert original mask
  typename OutputImageType::Pointer outimg = dil->GetOutput();

  itk::ImageRegionIteratorWithIndex<OutputImageType> it =
    itk::ImageRegionIteratorWithIndex<OutputImageType>(outimg, outimg->GetLargestPossibleRegion());

  for (it.GoToBegin(); !it.IsAtEnd(); ++it)
  {
    if (this->GetInput()->GetPixel(it.GetIndex()) != 0)
      it.Set(1);
  }

  this->GraftOutput(outimg);
}

/**
 * Standard "PrintSelf" method
 */
template <class TInputImage, class TOutput>
void
RandomDilateImageFilter<TInputImage, TOutput>
::PrintSelf(
  std::ostream& os, 
  itk::Indent indent) const
{
  Superclass::PrintSelf( os, indent );
  os << indent << "Radius: " << m_Radius << std::endl;
}

#endif
