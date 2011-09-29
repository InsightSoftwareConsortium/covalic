
#ifndef _LinearImageMetricAggregator_h
#define _LinearImageMetricAggregator_h

#include "ImageMetricAggregator.h"

#include <vector>

template <class TImage>
class ITK_EXPORT LinearImageMetricAggregator<TImage>:
  public ImageMetricAggregator<TImage>
{
public:

  typedef ImageMetricAggregator<TImage> Superclass;

  typedef typename Superclass::MetricType MetricType;
  typedef typename Superclass::MetricPointer MetricPointer;

  void AddMetricWeight(double w)
  { m_MetricWeights.push_back(w); }

  virtual double GetAggregateScore()
  {
    unsigned int numO = Superclass::m_MetricObjects.size();
    unsigned int numW = m_MetricWeights.size();

    if (numO != numW)
      itkExceptionMacro(<< "Number of objects and weights not the same");

    double score = 0;
    for (unsigned int i = 0; i < numW; i++)
    {
      MetricPointer obj = Superclass::m_MetricObjects[i];
      obj->SetFixedImage(m_FixedImage);
      obj->SetMovingImage(m_MovingImage);

      double w = m_MetricWeights[i];

      score += w * obj->GetValue();
    }

    return score;
  }

protected:

  std::vector<double> m_MetricWeights;

};

#endif
