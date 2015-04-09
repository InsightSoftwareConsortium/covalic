#
# Rank aggregation using unsupervised learning, to be used in ensemble
# analysis of perturbed segmentations/registrations
# (see demo at the end of file)
# 
# This is a modified version of:
# Klementiev, A., Roth, D., & Small, K. (2007). An unsupervised learning
# algorithm for rank aggregation. In Machine Learning: ECML 2007 (pp. 616-623).
#
# Author: Marcel Prastawa, April 2015
#

import numpy as np

class UnsupervisedLearningRankAggregator:

  def __init__(self):
    # Threshold, missing data/rank should be Inf
    self.threshold = 1000

    self.maxIterations = 1000

    self.learningRate = 0.5

    self.weights = None

  def get_weights(self):
    return self.weights

  def set_weights(self, W):
    self.weights = W / W.sum()

  def set_threshold(self, t):
    self.threshold = t

  def aggregate(self, metricTable, metricOrder):

    numSamples = metricTable.shape[0]
    numMetrics = metricTable.shape[1]

    self.weights = np.ones(numMetrics, np.float64) / numMetrics

    rankTable = np.zeros((numSamples, numMetrics), np.float64)
    for i in range(numMetrics):
      metricValues = metricTable[:,i].copy() # Smaller value is better
      if metricOrder[i] > 0:
        # Higher value is better
        metricValues *= -1.0

      rankTable[:,i] = np.argsort(metricValues)
      # TODO: if value = Inf mark rank as missing, also Inf

    #print "Ranks", rankTable

    for optIter in range(self.maxIterations):
      #print "Weights = ", self.weights

      numThresholded = np.sum(rankTable <= self.threshold)

      if numThresholded < 5:
        break

      meanRank = np.sum(rankTable, axis=1) / numThresholded

      #print "Mean rank", meanRank

      weightUpdates = np.ones(numMetrics, np.float64)

      for i in range(numMetrics):
        rank_i = rankTable[:,i]

        delta_i = (rank_i - meanRank)
        delta_i[rank_i > self.threshold] = \
          self.threshold + 1.0 - meanRank[rank_i > self.threshold]

        delta_i = delta_i ** 2.0

        weightUpdates[i] = np.exp(-self.learningRate * np.sum(delta_i))
      
      self.weights = weightUpdates / weightUpdates.sum()

    #print "Final weights = ", self.weights

  def get_aggregated_rank(self, metricTable, metricOrder):

    numSamples = metricTable.shape[0]
    numMetrics = metricTable.shape[1]

    Rweighted = np.zeros(numSamples, np.float64)
    for i in range(numMetrics):
      metricValues = metricTable[:,i].copy() # Smaller value is better
      if metricOrder[i] > 0:
        # Higher value is better
        metricValues *= -1.0

      Rweighted += self.weights[i] * np.argsort(metricValues)

    return np.argsort(Rweighted)


if __name__ == "__main__":

  trueRank = np.random.permutation(20)

  print "True rank", trueRank

  numSamples = len(trueRank)
  numMetrics = 5

  numSubSamples = int(numSamples * 0.8)

  metricValues = np.zeros((numSamples, numMetrics), np.float64)

  for i in range(numMetrics-1):
    #x = trueRank * 100 + np.random.randn(numSamples) * (i+1) * 0.5
    x = (numSamples - trueRank) * 100 + np.random.randn(numSamples) * (i+1) * 10
    metricValues[:,i] = np.round(x)
  metricValues[:,-1] = np.round( np.random.rand(numSamples) * 100 )


  # Create outlier
  #metricValues[7,1] = 20000

  metricOrder = np.ones(numMetrics)

  print "Metric values", metricValues

  averageWeights = np.zeros(numMetrics, np.float64)

  numPerturbations = 300

  for t in range(numPerturbations):
    sampleInd = np.random.permutation(numSamples)[:numSubSamples]

    metrics_t = np.zeros((numSubSamples, numMetrics), np.float64)
    for i in range(numMetrics-1):
      x = metricValues[sampleInd,i] + np.random.randn(numSubSamples) * (i+1) * 50
      metrics_t[:,i] = np.round(x)
    metrics_t[:,-1] = np.round( np.random.rand(numSubSamples) * 100 )

    rankagg = UnsupervisedLearningRankAggregator()
    rankagg.aggregate(metrics_t, metricOrder)

    #averageWeights += rankagg.get_weights()
    w = rankagg.get_weights()
    if t % 20 == 0:
      print w
    averageWeights += w

  averageWeights /= numPerturbations

  print "Average weights = ", averageWeights

  
  rankagg = UnsupervisedLearningRankAggregator()
  rankagg.set_weights(averageWeights)

  print "Aggregated rank\n", rankagg.get_aggregated_rank(metricValues, metricOrder)
  print "True rank\n", trueRank

  averageWeights[-1] += 0.2
  rankagg.set_weights(averageWeights)
  print "Aggregated rank with random put back in\n", rankagg.get_aggregated_rank(metricValues, metricOrder)
