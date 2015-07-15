#
# Rank aggregation using unsupervised learning, to be used in ensemble
# analysis of perturbed segmentations/registrations
# (see demo at the end of file)
# 
# This is a modified version of:
# Klementiev, A., Roth, D., & Small, K. (2007). An unsupervised learning
# algorithm for rank aggregation. In Machine Learning: ECML 2007 (pp. 616-623).
#
# Author: Marcel Prastawa
#

import numpy as np

import scipy.stats
import scipy.stats.mstats as mstats

class UnsupervisedLearningRankAggregator:

  def __init__(self):
    # Metric precision, number of decimals to be considered for ranking
    self.decimals = 8

    # Threshold for ranks, only look at top-k ranks for updates
    self.threshold = 10

    self.maxIterations = 100

    self.learningRate = 1e-4

    self.tolerance = 1e-6

    self.weights = None

  def set_decimals(self, n):
    if n < 1:
      raise Exception("Decimals must be >= 1")
    self.decimals = n

  def get_weights(self):
    return self.weights

  def set_weights(self, W):
    self.weights = W / W.sum()

  def set_threshold(self, t):
    """Set the threshold for rank updates, only look at top-k ranks."""
    self.threshold = t

  def get_rank_vector(self, x):
    """Get ranking with explicit handling of missing values, tagged as nan."""

    n = len(x)

    if np.all(np.isnan(x)):
      #return np.ones(len(x)) * n
      return np.ones(len(x)) * (self.threshold + 1)

    ranks = mstats.rankdata(np.ma.masked_invalid(x))

    maxrank = np.max(ranks)

    # Make all missing data have the same rank, not ordered by appearance
    #ranks[ranks == 0] = maxrank + 1
    ranks[ranks == 0] = self.threshold + 1


    """
    #ranks[ranks == 0] = n + 1
    ranks[ranks == 0] = np.nan

    # Convert nan to max rank, penalize missing submissions
    maxrank = np.nanmax(ranks)
    if np.isnan(maxrank):
      maxrank = 0 # All values missing, set to equal rank

    ranks[np.isnan(ranks)] = maxrank + 1
    """

    return ranks

  def aggregate(self, metricTableList, metricOrder):
    """ Compute weights that aggregate multiple rank metrics."""

    numTables = len(metricTableList)

    numSamples = metricTableList[0].shape[0]
    numMetrics = metricTableList[0].shape[1]

    self.weights = np.ones(numMetrics, np.float64) / numMetrics
    #self.weights = np.zeros(numMetrics, np.float64)

    rankTableList = []

    for metricTable in metricTableList:

      rankTable = np.zeros((numSamples, numMetrics), np.float64)

      for i in range(numMetrics):
        metricValues = metricTable[:,i].copy() # Smaller value is better
        if metricOrder[i] > 0:
          # Higher value is better
          metricValues *= -1.0

        # Handle missing values (nan) by filling with random values,
        # drawn from uniform distribution. This reduces reliability measure
        # of missing metrics.
        """
        minv = np.nanmin(metricValues)
        maxv = np.nanmax(metricValues)

        if np.isnan(maxv) or np.isnan(minv):
          minv = 0.0
          maxv = 1.0

        missingInd = np.where(np.isnan(metricValues))

        metricValues[missingInd] = np.random.uniform(minv, maxv,
          size=len(missingInd[0]))
        """

        # Round the values, consider insignificant bits to be equal
        metricValues = np.around(metricValues, decimals=self.decimals)

        rankTable[:,i] = self.get_rank_vector(metricValues)

      #print "Ranks", rankTable

      rankTableList.append(rankTable)

    for optIter in range(self.maxIterations):
      #print "Weights iter", optIter, " = ", np.around(self.weights, decimals=3)

      weightUpdates = self.weights.copy()

      gradw = np.zeros(numMetrics)

      #objF = 0

      for rankTable in rankTableList:

        numThresholded = \
          np.sum(rankTable <= self.threshold)

        if numThresholded < 5:
          continue

        rankTable[rankTable > self.threshold] = self.threshold + 1

        # Minimizing w * (r - centralRank)**2
        #centralRank = np.sum(rankTable, axis=1) / numThresholded

        centralRank = np.sum(rankTable, axis=1)
        centralRank = scipy.stats.rankdata(centralRank)

        #centralRank = np.median(rankTable, axis=1)
        #centralRank = scipy.stats.rankdata(centralRank)

        # Minimizing (r - sum(w * r))**2
        #centralRank = np.sum(rankTable * self.weights, axis=1)
        #centralRank = scipy.stats.rankdata(centralRank)

        #print "Mean rank", centralRank

        """
        delta = np.zeros(numSamples)
        for i in range(numMetrics):
          rank_i = rankTable[:,i]

          delta_i = (rank_i - centralRank)

          delta += delta_i

          #objF += np.sum(delta_i ** 2.0)
        """

        for i in range(numMetrics):
          rank_i = rankTable[:,i]

          delta_i = (rank_i - centralRank)

          delta_i = delta_i ** 2.0
          weightUpdates[i] *= np.exp(-self.learningRate * np.sum(delta_i))

          #ktau, pval = scipy.stats.kendalltau(rank_i, centralRank)
          #ktau = (ktau + 1) * -1 # Make it so high value means disagreement
          #weightUpdates[i] *= np.exp(-self.learningRate * ktau)

          #weightUpdates[i] *= np.exp(-self.learningRate * np.sum(delta * rank_i))

          #gradw[i] += np.sum(delta * rank_i)

          #objF += self.weights[i] * np.sum(delta_i)
          #objF += self.weights[i] * ktau

      #for i in range(numMetrics):
      #  weightUpdates[i] *= np.exp(-self.learningRate * gradw[i])
      
      prevWeights = self.weights

      self.weights = weightUpdates / weightUpdates.sum()

      #print "Objective", optIter, "=", objF

      if np.max(np.abs(self.weights - prevWeights)) < self.tolerance: 
        break

    #print "Final weights = ", self.weights

  def get_aggregated_rank(self, metricTable, metricOrder):
    """Get aggregated rank given current weight estimate."""

    if self.weights is None:
      raise Exception("Need to run aggregate() first.")

    numSamples = metricTable.shape[0]
    numMetrics = metricTable.shape[1]

    Rweighted = np.zeros(numSamples, np.float64)
    for i in range(numMetrics):
      metricValues = metricTable[:,i].copy() # Smaller value is better
      if metricOrder[i] > 0:
        # Higher value is better
        metricValues *= -1.0

      # Round the values, consider insignificant bits to be equal
      metricValues = np.around(metricValues, decimals=self.decimals)

      Ri = self.get_rank_vector(metricValues)
      Ri[np.isnan(Ri)] = len(metricValues)

      # Only do thresholding for updates, allow ranking to go beyond top-k
      #Ri[Ri > self.threshold] = self.threshold + 1

      Rweighted += self.weights[i] * Ri

    # Round average ranks since there are potential duplicates due to
    # missing data
    #Rweighted = np.around(Rweighted, decimals=1)

    return self.get_rank_vector(Rweighted)


if __name__ == "__main__":

  np.random.seed(78261310579)

  trueRank = np.random.permutation(20)

  print "True rank", trueRank

  numSamples = len(trueRank)
  numMetrics = 5

  numSubSamples = int(numSamples * 0.6)

  metricValues = np.zeros((numSamples, numMetrics), np.float64)

  for i in range(numMetrics-1):
    #x = trueRank * 100 + np.random.randn(numSamples) * (i+1) * 0.5
    x = (numSamples - trueRank) * 100 + np.random.randn(numSamples) * (i+1) * 10
    metricValues[:,i] = np.round(x)
  metricValues[:,-1] = np.round( np.random.rand(numSamples) * 100 )

  # Replace entries with outliers or missing values
  metricValues[7,0] = np.nan
  metricValues[10,2] = np.nan
  #metricValues[7,0] = 20000
  #metricValues[10,2] = 20000

  metricOrder = np.ones(numMetrics)

  print "Metric values", metricValues

  averageWeights = np.zeros(numMetrics, np.float64)

  numPerturbations = 100

  for t in range(numPerturbations):
    sampleInd = np.random.permutation(numSamples)[:numSubSamples]

    metrics_t = np.zeros((numSubSamples, numMetrics), np.float64)
    for i in range(numMetrics-1):
      x = metricValues[sampleInd,i] + np.random.randn(numSubSamples) * (i+1) * 50
      metrics_t[:,i] = np.round(x)
    metrics_t[:,-1] = np.round( np.random.rand(numSubSamples) * 100 )

    rankagg = UnsupervisedLearningRankAggregator()
    rankagg.aggregate([metrics_t], metricOrder)

    #averageWeights += rankagg.get_weights()
    w = rankagg.get_weights()
    #if t % 20 == 0:
    #  print w

    averageWeights += w

  averageWeights /= numPerturbations

  #print "Estimated weights = ", averageWeights
  print "Estimated weights = ", np.around(averageWeights, decimals=3)

  
  rankagg = UnsupervisedLearningRankAggregator()
  rankagg.set_weights(averageWeights)

  """
  x = np.random.rand(8)
  x[2] = np.nan
  x[5] = np.nan
  print "x", x
  print "argsort(x)", np.argsort(x)
  print "ranking(x)", rankagg.get_rank_vector(x)
  """

  print "True rank\n", np.float64(trueRank) + 1

  print "Aggregated rank with estimated weights\n", rankagg.get_aggregated_rank(metricValues, metricOrder)

  averageWeights[:] = 1.0
  averageWeights /= averageWeights.sum()
  rankagg.set_weights(averageWeights)
  print "Aggregated rank with equal weights\n", rankagg.get_aggregated_rank(metricValues, metricOrder)
