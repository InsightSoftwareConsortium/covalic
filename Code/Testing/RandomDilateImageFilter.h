
#ifndef __RandomDilateImageFilter_h
#define __RandomDilateImageFilter_h

#include "itkImageToImageFilter.h"
#include "itkImage.h"

template <class TInputImage, class TOutputImage>
class RandomDilateImageFilter :
    public itk::ImageToImageFilter< TInputImage, TOutputImage >
{
public:
  /** Extract dimension from input and output image. */
  itkStaticConstMacro(InputImageDimension, unsigned int,
                      TInputImage::ImageDimension);
  itkStaticConstMacro(OutputImageDimension, unsigned int,
                      TOutputImage::ImageDimension);

  /** Convenient typedefs for simplifying declarations. */
  typedef TInputImage InputImageType;
  typedef TOutputImage OutputImageType;

  /** Standard class typedefs. */
  typedef RandomDilateImageFilter Self;
  typedef itk::ImageToImageFilter< InputImageType, OutputImageType> Superclass;
  typedef itk::SmartPointer<Self> Pointer;
  typedef itk::SmartPointer<const Self>  ConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(RandomDilateImageFilter, itk::ImageToImageFilter);
  
  /** Image typedef support. */
  typedef typename InputImageType::PixelType InputPixelType;
  typedef typename OutputImageType::PixelType OutputPixelType;
  
  typedef typename InputImageType::RegionType InputImageRegionType;
  typedef typename OutputImageType::RegionType OutputImageRegionType;

  typedef typename InputImageType::SizeType InputSizeType;

  /** Set the radius of the neighborhood used to compute the mean. */
  itkSetMacro(Radius, InputSizeType);

  /** Get the radius of the neighborhood used to compute the mean */
  itkGetConstReferenceMacro(Radius, InputSizeType);

  void SetRadius(long n);
  
  /** MeanImageFilter needs a larger input requested region than
   * the output requested region.  As such, MeanImageFilter needs
   * to provide an implementation for GenerateInputRequestedRegion()
   * in order to inform the pipeline execution model.
   *
   * \sa ImageToImageFilter::GenerateInputRequestedRegion() */
  virtual void GenerateInputRequestedRegion() throw(itk::InvalidRequestedRegionError);

protected:
  RandomDilateImageFilter();
  virtual ~RandomDilateImageFilter() {}
  void PrintSelf(std::ostream& os, itk::Indent indent) const;

  void ThreadedGenerateData(const OutputImageRegionType& outputRegionForThread,
                            int threadId );

  void AfterThreadedGenerateData();

private:
  RandomDilateImageFilter(const Self&); //purposely not implemented
  void operator=(const Self&); //purposely not implemented

  InputSizeType m_Radius;
};
  
#ifndef MU_MANUAL_INSTANTIATION
#include "RandomDilateImageFilter.txx"
#endif

#endif
