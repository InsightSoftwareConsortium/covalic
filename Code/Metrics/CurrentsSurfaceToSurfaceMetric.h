
/*
  Currents metric
  Glaunes et al CVPR 2004

  NOTE: only for triangle meshes
*/

#ifndef _CurrentsSurfaceToSurfaceMetric_h
#define _CurrentsSurfaceToSurfaceMetric_h

#include "SurfaceToSurfaceMetric.h"

#include "vtkPolyData.h"

class CurrentsSurfaceToSurfaceMetric: public SurfaceToSurfaceMetric
{
public:

//TODO:
  // void UseParticleMeshApproximation();
  // void UseKdTreeApproximation();

  void SetKernelWidth(double d);
  double GetKernelWidth() const;

  virtual double GetValue();

protected:

  double ComputeCurrentsNorm(vtkPolyData* pd);

  vtkSmartPointer<vtkPolyData> ComputeWeightedPDNormals(vtkPolyData* polyData)

  double m_KernelWidth;
};

#endif
