
// Returns the average of the closest distance between points in S1 and S2

#ifndef _ClosestDistaceSurfaceToSurfaceMetric_h
#define _ClosestDistaceSurfaceToSurfaceMetric_h

#include "SurfaceToSurfaceMetric.h"

#include "vtkKdTreePointLocator.h"

class ClosestDistanceSurfaceToSurfaceMetric: public SurfaceToSurfaceMetric
{
public:

  virtual void SetFixedSurface(vtkPolyData* pd);

  virtual double GetValue();

protected:

  vtkSmartPointer<vtkKdTreePointLocator> m_FixedPointLocator;

};

#endif
