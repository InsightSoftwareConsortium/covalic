
/*
  Currents metric
  Glaunes et al CVPR 2004

  NOTE: only for triangle meshes
*/

#ifndef _CurrentsSurfaceToSurfaceMetric_h
#define _CurrentsSurfaceToSurfaceMetric_h

#include "SurfaceToSurfaceMetric.h"

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

  double m_KernelWidth;
};

#endif
