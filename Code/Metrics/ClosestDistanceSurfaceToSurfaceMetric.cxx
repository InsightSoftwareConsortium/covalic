
#include "ClosestDistanceSurfaceToSurfaceMetric.h"

#include <cmath>

void
ClosestDistanceSurfaceToSurfaceMetric
::SetFixedSurface(vtkPolyData* pd)
{
  SurfaceToSurfaceMetric::m_FixedSurface = pd;

  m_FixedPointLocator = vtkSmartPointer<vtkKdTreePointLocator>::New();
  m_FixedPointLocator->SetDataSet(pd);
  m_FixedPointLocator->BuildLocator();
}

ClosestDistanceSurfaceToSurfaceMetric::MeasureType
ClosestDistanceSurfaceToSurfaceMetric
::GetValue() const
{
  vtkPoints* fixedPts = this->GetFixedSurface()->GetPoints();

  vtkPoints* movingPts = this->GetMovingSurface()->GetPoints();

  if (movingPts->GetNumberOfPoints() == 0)
    return 0;

  double sumDist = 0;
  for (vtkIdType i = 0; i < movingPts->GetNumberOfPoints(); i++)
  {
    double x[3];
    movingPts->GetPoint(i, x);

    vtkIdType fixedId = m_FixedPointLocator->FindClosestPoint(x);    

    double y[3];
    fixedPts->GetPoint(fixedId, y);

    double dist_i = 0;
    for (int j = 0; j < 3; j++)
    {
      double d = x[j] - y[j];
      dist_i += d*d;
    }
    sumDist += sqrt(dist_i);
  }

  return sumDist / movingPts->GetNumberOfPoints();
};
