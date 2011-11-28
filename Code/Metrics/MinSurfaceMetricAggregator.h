
#ifndef _MinSurfaceMetricAggregator_h
#define _MinSurfaceMetricAggregator_h

#include "SurfaceMetricAggregator.h"

#include <vector>

class ITK_EXPORT MinSurfaceMetricAggregator<TSurface>:
  public SurfaceMetricAggregator
{
public:

  typedef SurfaceMetricAggregator Superclass;

  typedef typename Superclass::MetricType MetricType;
  typedef typename Superclass::MetricPointer MetricPointer;

  virtual double GetAggregateScore()
  {
    unsigned int numO = Superclass::m_MetricObjects.size();

    double minScore = 0;
    for (unsigned int i = 0; i < numO; i++)
    {
      MetricPointer obj = Superclass::m_MetricObjects[i];
      obj->SetFixedSurface(m_FixedSurface);
      obj->SetMovingSurface(m_MovingSurface);

      double score = obj->GetValue();

      if (i == 0)
      {
        minScore = score;
        continue;
      }

      if (score < minScore)
        minScore = score;
    }

    return minScore;
  }

protected:

};

#endif
