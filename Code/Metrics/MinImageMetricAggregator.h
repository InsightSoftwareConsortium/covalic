
#ifndef _MinImageMetricAggregator_h
#define _MinImageMetricAggregator_h

#include "ImageMetricAggregator.h"

#include <vector>

template <class TImage>
class ITK_EXPORT MinImageMetricAggregator<TImage>:
  public ImageMetricAggregator<TImage>
{
public:

  typedef ImageMetricAggregator<TImage> Superclass;

  typedef typename Superclass::MetricType MetricType;
  typedef typename Superclass::MetricPointer MetricPointer;

  virtual double GetAggregateScore()
  {
    unsigned int numO = Superclass::m_MetricObjects.size();

    double minScore = 0;
    for (unsigned int i = 0; i < numO; i++)
    {
      MetricPointer obj = Superclass::m_MetricObjects[i];
      obj->SetFixedImage(m_FixedImage);
      obj->SetMovingImage(m_MovingImage);

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
