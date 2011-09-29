
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

  typedef SurfaceToSurfaceMetric Superclass;
  typedef CurrentsSurfaceToSurfaceMetric Self;
  typedef itk::SmartPointer<Self> Pointer;
  typedef itk::SmartPointer<const Self>  ConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(CurrentsSurfaceToSurfaceMetric,
    SurfaceToSurfaceMetric);

  typedef Superclass::MeasureType MeasureType;
  typedef Superclass::ParametersType ParametersType;
  typedef Superclass::DerivativeType DerivativeType;

//TODO: Accomodate different approximation techniques
  // void UseParticleMeshApproximation();
  // void UseKdTreeApproximation();

  void SetKernelWidth(double d);
  double GetKernelWidth() const;

  virtual unsigned int GetNumberOfParameters() const
  { itkExceptionMacro(<< "Not implemented"); return 0; }

  virtual MeasureType GetValue() const;

  virtual MeasureType GetValue(const ParametersType& p) const
  { itkExceptionMacro(<< "Not implemented"); return 0; }

protected:

  CurrentsSurfaceToSurfaceMetric()
  {
    m_KernelWidth = 1.0;
  }

  double ComputeCurrentsNorm(vtkPolyData* pd) const;

  vtkSmartPointer<vtkPolyData> ComputeWeightedPDNormals(vtkPolyData* polyData) const;

  double m_KernelWidth;
};

#endif
