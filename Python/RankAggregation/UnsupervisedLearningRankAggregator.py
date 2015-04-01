
import numpy as np

class UnsupervisedLearningRankAggregator:

  def __init__(self):
    # Threshold, missing data/rank should be Inf
    self.threshold = 1000

    self.maxIterations = 500

    self.learningRate = 0.25

    self.weights = None

  def get_weights(self):
    return self.weights

  def set_weights(self, W):
    self.weights = W

  def set_threshold(self, t):
    self.threshold = t

  def aggregate(self, metricTable, metricOrder):

    numSamples = metricTable.shape[0]
    numMetrics = metricTable.shape[1]

    self.weights = np.ones(numMetrics, np.float64) / numMetrics

    rankTable = np.zeros((numSamples, numMetrics), np.float64)
    for i in range(numMetrics):
      metricValues = metricTable[:,i].copy() * -1.0 # Higher value is better
      if metricOrder[i] < 0:
        # Smaller value is better
        metricValues *= -1.0

      rankTable[:,i] = np.argsort(metricValues)
      # if value = Inf mark rank as missing

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

    print "Final weights = ", self.weights

  def get_aggregated_rank(self, metricTable, metricOrder):

    numSamples = metricTable.shape[0]
    numMetrics = metricTable.shape[1]

    Rweighted = np.zeros(numSamples, np.float64)
    for i in range(numMetrics):
      metricValues = metricTable[:,i].copy() * -1.0 # Higher value is better
      if metricOrder[i] < 0:
        # Smaller value is better
        metricValues *= -1.0

      Rweighted += self.weights[i] * np.argsort(metricValues)

    return np.argsort(Rweighted)


if __name__ == "__main__":

  trueRank = np.random.permutation(20)

  print "True rank", trueRank

  numSamples = len(trueRank)
  numMetrics = 5

  metricValues = np.zeros((numSamples, numMetrics), np.float64)

  for i in range(numMetrics-1):
    #x = trueRank * 100 + np.random.randn(numSamples) * (i+1) * 0.5
    x = (numSamples - trueRank) * 100 + np.random.randn(numSamples) * (i+1) * 20
    metricValues[:,i] = np.round(x)
  metricValues[:,-1] = np.round( np.random.rand(numSamples) * 100 )

  metricOrder = np.ones(numMetrics)

  print "Metric values", metricValues

  ensembleWeights = np.zeros(numMetrics, np.float64)

  numEnsemble = 100

  for t in range(numEnsemble):

    metrics_t = metricValues.copy()
    for i in range(numMetrics-1):
      x = metrics_t[:,i] + np.random.randn(numSamples) * (i+1) * 100
      metrics_t[:,i] = np.round(x)
    metrics_t[:,-1] = np.round( np.random.rand(numSamples) * 100 )

    rankagg = UnsupervisedLearningRankAggregator()
    rankagg.aggregate(metrics_t, metricOrder)

    ensembleWeights += rankagg.get_weights()

  ensembleWeights /= numEnsemble

  print "Final ensemble weights", ensembleWeights
  
  rankagg = UnsupervisedLearningRankAggregator()
  rankagg.set_weights(ensembleWeights)

  print "Aggregated rank", rankagg.get_aggregated_rank(metricValues, metricOrder)
  print "True rank", trueRank
