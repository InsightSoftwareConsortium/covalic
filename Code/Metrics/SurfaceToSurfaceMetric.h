#ifndef _SurfaceToSurfaceMetric_h
#define _SurfaceToSurfaceMetric_h

#include "itkSingleValuedCostFunction.h"

#include "vtkPolyData.h"

class SurfaceToSurfaceMetric: public itk::SingleValuedCostFunction
{
public:
  /** Standard class typedefs. */
  typedef SurfaceToSurfaceMetric         Self;
  typedef SingleValuedCostFunction   Superclass;
  typedef SmartPointer< Self >       Pointer;
  typedef SmartPointer< const Self > ConstPointer;

  /** Run-time type information (and related methods). */
  itkTypeMacro(ImageToImageMetric, SingleValuedCostFunction);

  virtual void SetFixedSurface(vtkPolyData* pd);
  vtkPolyData* GetFixedSurface() const;

  virtual void SetMovingSurface(vtkPolyData* pd);
  vtkPolyData* GetMovingSurface() const;

// TODO:
  //void SetTransformParameters(p);

  virtual double GetValue() = 0;

protected:

  vtkPolyData* m_FixedSurface;
  vtkPolyData* m_MovingSurface;
};

#endif
