
#ifndef _SurfaceMetricAggregator_h
#define _SurfaceMetricAggregator_h

#include "itkObject.h"

#include "vtkPolyData.h"
#include "vtkSmartPointer.h"

#include "SurfaceToSurfaceMetric.h"

#include <vector>

class SurfaceMetricAggregator: public itk::Object
{
public:

  typedef vtkPolyData SurfaceType;
  typedef vtkSmartPointer<vtkPolyData> SurfacePointer;

  typedef SurfaceToSurfaceMetric MetricType;
  typedef typename MetricType::Pointer MetricPointer;

  void SetFixedSurface(const SurfaceType* img)
  { m_FixedSurface = img; }
  void SetMovingSurface(const SurfaceType* img)
  { m_MovingSurface = img; }

  void AddMetricObject(const MetricType* obj)
  { m_MetricObjects.push_back(obj); }

  virtual double GetAggregateScore() = 0;

protected:

  vtkSmartPointer<vtkPolyData> m_FixedSurface;
  vtkSmartPointer<vtkPolyData> m_MovingSurface;

  std::vector<MetricPointer> m_MetricObjects;

};

#endif
