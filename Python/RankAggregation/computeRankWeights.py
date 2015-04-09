#! /usr/bin/env python
#
# Rank aggregation of image segmentation fidelity to ground truth using
# unsupervised learning combined with ensemble analysis of perturbed 
# segmentations/registrations
# 
# Based on
# Klementiev, A., Roth, D., & Small, K. (2007). An unsupervised learning
# algorithm for rank aggregation. In Machine Learning: ECML 2007 (pp. 616-623).
#
# Author: Marcel Prastawa, April 2015
#

# NOTE: THIS IS STILL A PSEUDOCODE

from UnsupervisedLearningRankAggregator import UnsupervisedLearningRankAggregator

import numpy as np

import os
import sys

import subprocess
import tempfile

if __name__ == "__main__":


  if len(sys.argv) != 4:
    print "Usage:", sys.argv[0], " <binary path> <ground truth path> <submissions path>"
    sys.exit(-1)

  binaryPath = sys.argv[1]
  groundTruthPath = sys.argv[2]
  submissionsRootPath = sys.argv[3]

  metricBinaryList = sorted(glob.glob(os.path.join(binaryPath, "ValidateImage*")))

  if not metricBinaryList:
    print "No image metrics detected"
    sys.exit(-1)

  numMetrics = len(metricBinaryList)

  # Order of metric for ranking (high is good = 1, high is bad = -1)
  # Distance measures are marked negative
  metricOrder = np.ones(numMetrics)
  for i in range(metricBinaryList):
    if metricBinarList[i].find("Distance"):
      metricOrder[i] = -1

  perturbBinaryList = glob.glob(os.path.join(binaryPath, "ValidateImage*"))

  if not perturbBinaryList:
    print "No image perturbation apps detected"
    sys.exit(-1)

  numPerturbers = len(perturbBinaryList)

  # Get ground truth and submission files
  groundTruthFiles = \
    [f for f in sorted(glob.glob(os.path.join(groundTruthPath, "*"))) \
     if os.path.isfile(f)]

  submissionFilesTable = []
  for p in sorted(glob.glob(os.path.join(submissionsRootPath, "*"))):
    if os.path.isdir(p):
      submissionFiles = \
        [f for f in sorted(glob.glob(os.path.join(submissionsRootPath, p, "*"))) \
         if os.path.isfile(f)]
      if len(submissionFiles) == len(groundTruthFiles):
        submissionFilesTable.append(submissionFiles)
      else:
        raise Exception("WARNING: submissions in", p,
          "does not match file count of ground truth")

  numSamples = len(submissionFilesTable)

  numSubSamples = int(numSamples * 0.8)

  # Compute average weights with different perturbations and bagging
  averageWeights = np.zeros(numMetrics, np.float64)

  numPerturbations = 300

  for t in range(numPerturbations):
    sampleInd = np.random.permutation(numSamples)[:numSubSamples]

    metricValues = []

    for i in sampleInd:

      submissionFiles = submissionFilesTable[i]

      # Pick a type of perturbation
      perturber = perturbBinaryList[np.random.randint(0, numPerturbers)]

      for q in range(len(submissionFiles)):
        gt = groundTruthFiles[q]
        subm = submissionFiles[q]

        # Output perturbed image to temp dir
        psubm = os.path.join(tempfile.gettempdir(), "pert_" + subm)

        cmd = (perturber, subm, psubm) 

        p = subprocess.open(args=command, stdout=subprocess.PIPE,
          stderr=subprocess.PIPE)
        stdout, stderr = p.communicate()

        # Evaluate each metric on gt and perturbed submission
        entry = []
        for j in range(numMetrics):

          cmd = (metricBinaryList[j], gt, psubm)

          p = subprocess.open(args=command, stdout=subprocess.PIPE,
            stderr=subprocess.PIPE)
          stdout, stderr = p.communicate()

          #TODO: obtain list of metrics from stdout
          entry.append(getMetricValues(stdout))

        metricValues.append(entry)

      metricValues = np.array(metricValues, np.float64)

      rankagg = UnsupervisedLearningRankAggregator()
      rankagg.aggregate(metricValues, metricOrder)

      #averageWeights += rankagg.get_weights()
      w = rankagg.get_weights()
      if t % 20 == 0:
        print w
      averageWeights += w

  averageWeights /= numPerturbations

  print "Average weights = ", averageWeights

  # TODO: store average weights for future evaluation
