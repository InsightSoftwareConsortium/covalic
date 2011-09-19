
// Returns the Haussdorff distance between points in S1 and S2
// Defined as max(max closest d(S1, S2), max closest d(S2, S1))

#ifndef _HaussdorffDistaceSurfaceToSurfaceMetric_h
#define _HaussdorffDistaceSurfaceToSurfaceMetric_h

#include "SurfaceToSurfaceMetric.h"

#include "vtkKdTreePointLocator.h"

class HaussdorffDistanceSurfaceToSurfaceMetric: public SurfaceToSurfaceMetric
{
public:

  virtual void SetFixedSurface(vtkPolyData* pd);
  virtual void SetMovingSurface(vtkPolyData* pd);

  virtual double GetValue();

protected:

  vtkSmartPointer<vtkKdTreePointLocator> m_FixedPointLocator;
  vtkSmartPointer<vtkKdTreePointLocator> m_MovingPointLocator;

};

#endif
