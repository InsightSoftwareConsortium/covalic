
#include "CurrentsSurfaceToSurfaceMetric.h"

void
CurrentsSurfaceToSurfaceMetric
::SetKernelWidth(double d)
{
  m_KernelWidth = d;
}

double
CurrentsSurfaceToSurfaceMetric
::GetKernelWidth()
{
  return m_KernelWidth;
}

double
CurrentsSurfaceToSurfaceMetric
::GetValue()
{
//TODO:
  // if (m_Approximation == ParticleMesh)
  //   return this->GetValueWithParticleMesh();

  vtkPolyData* movingPD = this->GetMovingSurface();
  vtkPolyData* fixedPD = this->GetFixedSurface();

//TODO

  // Compute triangle centroids: c

  // Compute weighted normals: n

  // sum_i sum_j k(c_i, c_j) <n_i, n_j>
}
