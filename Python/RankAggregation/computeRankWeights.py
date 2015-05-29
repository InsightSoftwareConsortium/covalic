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
# Author: Marcel Prastawa
#

from UnsupervisedLearningRankAggregator import UnsupervisedLearningRankAggregator

import SimpleITK as sitk

import numpy as np

import os, sys
import glob

import subprocess
import tempfile

def getMetricValues(textFilename, numLabels):
  """Get list of metric values from a text file."""

  f = open(textFilename, 'r')

  # Tag missing segmentation objects with infinity
  metrics = [np.inf] * numLabels

  for line in f:
    name, value = line.split('=')

    inner = name.split("(")[1]
    item  = inner.split(",")[0]

    if len(item.split("_")) < 2:
      # Single entry for multiple objects (e.g., kappa)
      return [float(value)]

    index = int( item.split("_")[1] ) - 1

    metrics[index] = float(value)

  #print "Metric values for",  metrics
  return metrics

def getSubjectKey(s):
  """Get subject key given the filename."""
  f, ext = os.path.splitext(os.path.basename(s))
  # Assume names are split between subject ID and submission ID by underscore
  s = f.split("_")[0]
  # TODO: generalize this for other naming conventions?
  return s

if __name__ == "__main__":

  if len(sys.argv) != 4:
    print "Usage:", sys.argv[0], " <binary path> <ground truth path> <submissions path>"
    sys.exit(-1)

  binaryPath = sys.argv[1]
  groundTruthPath = sys.argv[2]
  submissionsRootPath = sys.argv[3]

  metricBinaryList = sorted(glob.glob(os.path.join(binaryPath, "ValidateImage*")))

  # TODO DEBUG
  # Take out special cases
  metricBinaryList = [m for m in metricBinaryList if m.find("Kappa") < 0]

  if not metricBinaryList:
    print "No image metric apps detected"
    sys.exit(-1)

  numMetrics = len(metricBinaryList)

  perturbBinaryList = glob.glob(os.path.join(binaryPath, "PerturbImageLabels*"))

  if not perturbBinaryList:
    print "No image perturbation apps detected"
    sys.exit(-1)

  numPerturbers = len(perturbBinaryList)

  # Get ground truth and submission files
  groundTruthFiles = \
    [f for f in sorted(glob.glob(os.path.join(groundTruthPath, "*"))) \
     if os.path.isfile(f)]

  # Compute number of objects in ground truth set
  numObjects = 0
  for f in groundTruthFiles:
    labelImage = sitk.ReadImage(f)
    labelArray = sitk.GetArrayFromImage(labelImage)
    maxLabel = np.max(labelArray)
    if maxLabel > numObjects:
      numObjects = maxLabel

  print "Analyzing", numObjects, "objects in all label images"

  # Order of metric for ranking (high is good = 1, high is bad = -1)
  # Distance measures are marked negative
  metricOrder = []
  for i in range(numMetrics):
    if metricBinaryList[i].find("Distance") >= 0:
      metricOrder += [-1] * numObjects
    elif metricBinaryList[i].find("Kappa") >= 0:
      metricOrder += [1]
    else:
      metricOrder += [1] * numObjects
  metricOrder = np.array(metricOrder, np.float64)

  submissionFilesTable = []
  for p in sorted(glob.glob(os.path.join(submissionsRootPath, "*"))):
    if os.path.isdir(p):

      # Select files that correspond to a ground truth
      submissionFiles = []
      for f in sorted(glob.glob(os.path.join(submissionsRootPath, p, "*"))):
        if not os.path.isfile(f):
          continue

        q = getSubjectKey(f)

        match = False
        for t in groundTruthFiles:
          if os.path.basename(t).find(q) >= 0:
            match = True
            break

        if match:
          submissionFiles.append(f)

      # Check for completeness
      # TODO: allow incomplete submissions?
      if len(submissionFiles) == len(groundTruthFiles):
        submissionFilesTable.append(submissionFiles)
      else:
        raise Exception("WARNING: submissions in", p,
          "does not match file count of ground truth")

  numSamples = len(submissionFilesTable)

  numSubSamples = int(numSamples * 0.8)

  # Compute average weights with different perturbations and bagging
  # represents multiple metrics and multiple objects
  averageWeights = np.zeros(numMetrics * numObjects, np.float64)

  numPerturbations = 5

  # TODO: rank each case separately compute average weights for different
  # perturbations, then average them
  # OR
  # pool samples based on algorithm-cases? each an observation?
  for t in range(numPerturbations):
    #sampleInd = np.random.permutation(numSamples)[:numSubSamples]
    sampleInd = range(numSamples)

    for i in sampleInd:

      metricTable = []

      submissionFiles = submissionFilesTable[i]

      # Pick a type of perturbation
      perturber = perturbBinaryList[np.random.randint(0, numPerturbers)]

      print "Applying", os.path.basename(perturber)

      for q in range(len(groundTruthFiles)):
        gt = groundTruthFiles[q]


        # Search for match to this ground truth in submissions list
        subm = None
        for f in submissionFiles:
          q = getSubjectKey(f)

          if os.path.basename(gt).find(q) >= 0:
            subm = f

        # Handle missing submissions
        if subm is None:
          # TODO
          #missingEntry = np.ones((numMetrics,), np.float64)
          #missingEntry[:] = np.inf
          #metricValues.append(missingEntry)
          raise Exception("Missing submissions for", gt)

        # Output perturbed image to temp dir
        psubm = os.path.join(tempfile.gettempdir(),
          "pert_" + os.path.basename(subm))

        textout = os.path.join(tempfile.gettempdir(), psubm + ".out")

        # TODO
        # Custom parameters for each perturbation type? Use defaults for now
        command = (perturber, subm, psubm)

        #print "Running", command

        p = subprocess.Popen(args=command, stdout=subprocess.PIPE,
          stderr=subprocess.PIPE)
        stdout, stderr = p.communicate()

        # Evaluate each metric on gt and perturbed submission
        metricValues = []
        for j in range(numMetrics):

          # Run validation app and obtain list of metrics from stdout
          command = (metricBinaryList[j], gt, psubm, textout)

          #print "Running", command

          p = subprocess.Popen(args=command, stdout=subprocess.PIPE,
            stderr=subprocess.PIPE)
          stdout, stderr = p.communicate()

          metricValues += getMetricValues(textout, numObjects)

        metricTable.append(metricValues)

      metricTable = np.array(metricTable, np.float64)

      print metricTable.shape
      print metricTable

      rankagg = UnsupervisedLearningRankAggregator()
      rankagg.aggregate(metricTable, metricOrder)

      #averageWeights += rankagg.get_weights()
      w = rankagg.get_weights()
      if t % 20 == 0:
        print w
      averageWeights += w

  #TODO?
  #averageWeights /= numSubSamples
  averageWeights /= numSamples

  averageWeights /= numPerturbations

  print "Average weights = ", averageWeights

  # Store average weights for future evaluation
  np.save("metric_weights.npy", average_weights)
