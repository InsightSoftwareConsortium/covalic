
#include "HaussdorffDistanceSurfaceToSurfaceMetric.h"

#include <cmath>

void
HaussdorffDistanceSurfaceToSurfaceMetric
::SetFixedSurface(vtkPolyData* pd)
{
  SurfaceToSurfaceMetric::m_FixedSurface = pd;

  m_FixedPointLocator = vtkSmartPointer<vtkKdTreePointLocator>::New();
  m_FixedPointLocator->SetDataSet(pd);
  m_FixedPointLocator->BuildLocator();
}

void
HaussdorffDistanceSurfaceToSurfaceMetric
::SetMovingSurface(vtkPolyData* pd)
{
  SurfaceToSurfaceMetric::m_MovingSurface = pd;

  m_MovingPointLocator = vtkSmartPointer<vtkKdTreePointLocator>::New();
  m_MovingPointLocator->SetDataSet(pd);
  m_MovingPointLocator->BuildLocator();
}

HaussdorffDistanceSurfaceToSurfaceMetric::MeasureType
HaussdorffDistanceSurfaceToSurfaceMetric
::GetValue() const
{
  vtkPoints* fixedPts = this->GetFixedSurface()->GetPoints();

  vtkPoints* movingPts = this->GetMovingSurface()->GetPoints();

  if (movingPts->GetNumberOfPoints() == 0)
    return 0;

  double maxDist1 = 0;
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
    dist_i = sqrt(dist_i);

    if (dist_i > maxDist1)
      maxDist1 = dist_i;
  }

  double maxDist2 = 0;
  for (vtkIdType i = 0; i < fixedPts->GetNumberOfPoints(); i++)
  {
    double x[3];
    fixedPts->GetPoint(i, x);

    vtkIdType movingId = m_MovingPointLocator->FindClosestPoint(x);    

    double y[3];
    movingPts->GetPoint(movingId, y);

    double dist_i = 0;
    for (int j = 0; j < 3; j++)
    {
      double d = x[j] - y[j];
      dist_i += d*d;
    }
    dist_i = sqrt(dist_i);

    if (dist_i > maxDist1)
      maxDist2 = dist_i;
  }

  double H = (maxDist1 > maxDist2) ? maxDist1 : maxDist2;

  return H;
};
