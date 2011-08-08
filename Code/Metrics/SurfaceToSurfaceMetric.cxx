
#include "SurfaceToSurfaceMetric.h"

void
SurfaceToSurfaceMetric
::SetFixedSurface(vtkPolyData* pd)
{
  m_FixedSurface = pd;
}

vtkPolyData*
SurfaceToSurfaceMetric
::GetFixedSurface() const
{
  return m_FixedSurface;
}

void
SurfaceToSurfaceMetric
::SetMovingSurface(vtkPolyData* pd)
{
  m_MovingSurface = pd;
}

vtkPolyData*
SurfaceToSurfaceMetric
::GetMovingSurface() const
{
  return m_MovingSurface;
}
