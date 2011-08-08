
#ifndef _ClosestDistaceSurfaceToSurfaceMetric_h
#define _ClosestDistaceSurfaceToSurfaceMetric_h

#include "SurfaceToSurfaceMetric.h"

#include "vtkKdTreePointLocator.h"

class ClosestDistanceSurfaceToSurfaceMetric: public SurfaceToSurfaceMetric
{
public:
  virtual void SetSurfaceImage(vtkPolyData* pd);

  virtual double GetValue();

protected:

  vtkSmartPointer<vtkKdTreePointLocator> m_FixedPointLocator;
};

#endif
