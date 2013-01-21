
#include "HausdorffDistanceSurfaceToSurfaceMetric.h"

#include <algorithm>
#include <cmath>
#include <vector>

#include "vnl/vnl_math.h"

void
HausdorffDistanceSurfaceToSurfaceMetric
::SetFixedSurface(vtkPolyData* pd)
{
  SurfaceToSurfaceMetric::m_FixedSurface = pd;

  m_FixedPointLocator = vtkSmartPointer<vtkKdTreePointLocator>::New();
  m_FixedPointLocator->SetDataSet(pd);
  m_FixedPointLocator->BuildLocator();
}

void
HausdorffDistanceSurfaceToSurfaceMetric
::SetMovingSurface(vtkPolyData* pd)
{
  SurfaceToSurfaceMetric::m_MovingSurface = pd;

  m_MovingPointLocator = vtkSmartPointer<vtkKdTreePointLocator>::New();
  m_MovingPointLocator->SetDataSet(pd);
  m_MovingPointLocator->BuildLocator();
}

void
HausdorffDistanceSurfaceToSurfaceMetric
::SetPercentile(double p)
{
  if (p < 0.0 || p > 1.0)
    itkExceptionMacro("Percentile needs to be in [0,1]");

  m_Percentile = p;
}

HausdorffDistanceSurfaceToSurfaceMetric::MeasureType
HausdorffDistanceSurfaceToSurfaceMetric
::GetValue() const
{
  vtkPoints* fixedPts = this->GetFixedSurface()->GetPoints();

  vtkPoints* movingPts = this->GetMovingSurface()->GetPoints();

  if (movingPts->GetNumberOfPoints() == 0 || fixedPts->GetNumberOfPoints() == 0)
  {
    if (movingPts->GetNumberOfPoints() == fixedPts->GetNumberOfPoints())
      return 0;
    else
      return vnl_huge_val(1.0);
  }

  std::vector<double> distances1;
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

    distances1.push_back(dist_i);
  }

  std::vector<double> distances2;
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

    distances2.push_back(dist_i);
  }

  double H1 = distances1[(int)(m_Percentile*(distances1.size()-1))];
  double H2 = distances2[(int)(m_Percentile*(distances2.size()-1))];

  return (H1 > H2) ? H1 : H2;
};
