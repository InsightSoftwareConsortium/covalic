
// Returns the average of the closest distance between points in S1 and S2

#ifndef _ClosestDistaceSurfaceToSurfaceMetric_h
#define _ClosestDistaceSurfaceToSurfaceMetric_h

#include "SurfaceToSurfaceMetric.h"

#include "vtkKdTreePointLocator.h"
#include "vtkSmartPointer.h"

class ClosestDistanceSurfaceToSurfaceMetric: public SurfaceToSurfaceMetric
{

public:

  typedef SurfaceToSurfaceMetric Superclass;
  typedef ClosestDistanceSurfaceToSurfaceMetric Self;
  typedef itk::SmartPointer<Self> Pointer;
  typedef itk::SmartPointer<const Self>  ConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(ClosestDistanceSurfaceToSurfaceMetric,
    SurfaceToSurfaceMetric);

  typedef Superclass::MeasureType MeasureType;
  typedef Superclass::ParametersType ParametersType;
  typedef Superclass::DerivativeType DerivativeType;

  virtual void SetFixedSurface(vtkPolyData* pd);

  virtual unsigned int GetNumberOfParameters() const
  { itkExceptionMacro(<< "Not implemented"); return 0; }

  MeasureType GetValue() const;

  MeasureType GetValue(const ParametersType& p) const
  { itkExceptionMacro(<< "Not implemented"); return 0; }

protected:

  vtkSmartPointer<vtkKdTreePointLocator> m_FixedPointLocator;

};

#endif
