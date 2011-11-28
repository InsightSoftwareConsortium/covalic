
#ifndef _MaxSurfaceMetricAggregator_h
#define _MaxSurfaceMetricAggregator_h

#include "SurfaceMetricAggregator.h"

#include <vector>

class ITK_EXPORT MaxSurfaceMetricAggregator<TSurface>:
  public SurfaceMetricAggregator
{
public:

  typedef SurfaceMetricAggregator Superclass;

  typedef typename Superclass::MetricType MetricType;
  typedef typename Superclass::MetricPointer MetricPointer;

  virtual double GetAggregateScore()
  {
    unsigned int numO = Superclass::m_MetricObjects.size();

    double maxScore = 0;
    for (unsigned int i = 0; i < numO; i++)
    {
      MetricPointer obj = Superclass::m_MetricObjects[i];
      obj->SetFixedSurface(m_FixedSurface);
      obj->SetMovingSurface(m_MovingSurface);

      double score = obj->GetValue();

      if (i == 0)
      {
        maxScore = score;
        continue;
      }

      if (score > maxScore)
        maxScore = score;
    }

    return maxScore;
  }

protected:

};

#endif
