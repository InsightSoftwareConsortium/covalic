
// Returns the Haussdorff distance between points in S1 and S2
// Defined as max(max closest d(S1, S2), max closest d(S2, S1))

#ifndef _HaussdorffDistaceSurfaceToSurfaceMetric_h
#define _HaussdorffDistaceSurfaceToSurfaceMetric_h

#include "SurfaceToSurfaceMetric.h"

#include "vtkKdTreePointLocator.h"
#include "vtkSmartPointer.h"

class HaussdorffDistanceSurfaceToSurfaceMetric: public SurfaceToSurfaceMetric
{
public:

  typedef SurfaceToSurfaceMetric Superclass;
  typedef HaussdorffDistanceSurfaceToSurfaceMetric Self;
  typedef itk::SmartPointer<Self> Pointer;
  typedef itk::SmartPointer<const Self>  ConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(HaussdorffDistanceSurfaceToSurfaceMetric,
    SurfaceToSurfaceMetric);

  typedef Superclass::MeasureType MeasureType;
  typedef Superclass::ParametersType ParametersType;
  typedef Superclass::DerivativeType DerivativeType;

  virtual void SetFixedSurface(vtkPolyData* pd);
  virtual void SetMovingSurface(vtkPolyData* pd);

  virtual unsigned int GetNumberOfParameters() const
  { itkExceptionMacro(<< "Not implemented"); return 0; }

  virtual MeasureType GetValue() const;

  virtual MeasureType GetValue(const ParametersType& p) const
  { itkExceptionMacro(<< "Not implemented"); return 0; }

protected:

  vtkSmartPointer<vtkKdTreePointLocator> m_FixedPointLocator;
  vtkSmartPointer<vtkKdTreePointLocator> m_MovingPointLocator;

};

#endif
