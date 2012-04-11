#ifndef _SurfaceToSurfaceMetric_h
#define _SurfaceToSurfaceMetric_h

#include "itkSingleValuedCostFunction.h"

#include "vtkPolyData.h"
#include "vtkSmartPointer.h"

class SurfaceToSurfaceMetric: public itk::SingleValuedCostFunction
{
public:
  /** Standard class typedefs. */
  typedef SurfaceToSurfaceMetric         Self;
  typedef itk::SingleValuedCostFunction   Superclass;
  typedef itk::SmartPointer< Self >       Pointer;
  typedef itk::SmartPointer< const Self > ConstPointer;

  typedef Superclass::MeasureType MeasureType;
  typedef Superclass::ParametersType ParametersType;
  typedef Superclass::DerivativeType DerivativeType;

  /** Run-time type information (and related methods). */
  itkTypeMacro(ImageToImageMetric, SingleValuedCostFunction);

  virtual void SetFixedSurface(vtkPolyData* pd);
  vtkPolyData* GetFixedSurface() const;

  virtual void SetMovingSurface(vtkPolyData* pd);
  vtkPolyData* GetMovingSurface() const;

// TODO:
  //void SetTransformParameters(p);

  virtual MeasureType GetValue() const = 0;

  virtual MeasureType GetValue(const ParametersType& p) const = 0;

  virtual void GetDerivative(const ParametersType& p, DerivativeType& dp) const 
  { itkExceptionMacro(<< "Not implemented"); }

  virtual void GetValueAndDerivative(const ParametersType& p, MeasureType& v, DerivativeType& dp) const
  { itkExceptionMacro(<< "Not implemented"); }

protected:

  vtkSmartPointer<vtkPolyData> m_FixedSurface;
  vtkSmartPointer<vtkPolyData> m_MovingSurface;
};

#endif
