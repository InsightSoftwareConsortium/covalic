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

import random

import shutil
import subprocess
import tempfile

def getMetricValues(textFilename, numLabels):
  """Get list of metric values from a text file."""

  f = open(textFilename, 'r')

  # Tag missing segmentation objects using nan
  metrics = [np.nan] * numLabels

  for line in f:
    name, value = line.split('=')

    inner = name.split("(")[1]
    item  = inner.split(",")[0]

    value = float(value)
    # For consistency, tag inf as nan 
    if np.isinf(value):
      value = np.nan

    if len(item.split("_")) < 2:
      # Single entry for multiple objects (e.g., kappa)
      return [value]

    index = int( item.split("_")[1] ) - 1

    metrics[index] = value

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
  #metricBinaryList = [m for m in metricBinaryList if m.find("Kappa") < 0]

  # DEBUG
  debugAddRandom = False
  if debugAddRandom:
    random.seed(279075032630680)

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
    if metricBinaryList[i].find("Kappa") >= 0:
      metricOrder += [1] # Only one value for entire object set
    elif metricBinaryList[i].find("Dist") >= 0:
      metricOrder += [-1] * numObjects # Smaller is better
    else:
      metricOrder += [1] * numObjects # Default, larger is better

  if debugAddRandom:
    metricOrder += [1]

  metricOrder = np.array(metricOrder, np.float64)

  # Name of each element in evaluation (by metric-object)
  metricNames = []
  for i in range(numMetrics):
    m = os.path.basename(metricBinaryList[i])
    m = m[len("ValidateImage"):]
    if metricBinaryList[i].find("Kappa") >= 0:
      metricNames += [m + "_all"]
    else:
      for j in range(numObjects):
        metricNames += [m + "_object" + str(j+1)]

  if debugAddRandom:
    metricNames += ["Random"]

  print "Metric names:\n", metricNames
  print "Metric order:\n", metricOrder

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

      # Check for zero entries in a submission path
      if len(submissionFiles) == 0:
        raise Exception("Zero submissions found in " + p)

      submissionFilesTable.append(submissionFiles)

  numSamples = len(submissionFilesTable)

  numSubSamples = int(numSamples * 0.8)

  # Compute average weights with different perturbations and bagging
  # represents multiple metrics and multiple objects
  averageWeights = np.zeros((len(metricNames),), np.float64)

  numPerturbations = 10

  tempFileSet = set()

  # TODO: rank each case separately compute average weights for different
  # perturbations, then average them
  # OR
  # pool samples based on algorithm-cases? each an observation?
  for t in range(numPerturbations):
    sampleInd = np.random.permutation(numSamples)[:numSubSamples]
    #sampleInd = range(numSamples)

    for i in sampleInd:

      metricTable = []

      submissionFiles = submissionFilesTable[i]

      # Pick a type of perturbation
      j = np.random.randint(0, numPerturbers+1)
      if j < numPerturbers:
        perturber = perturbBinaryList[j]
      else:
        perturber = "PassThrough"

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
          print "WARNING: Missing submissions for", gt, \
            "in path", os.path.dirname(submissionFiles[0])
          metricTable.append( [np.nan] * len(metricNames) )
          continue

        # Output perturbed image to temp dir
        psubm = os.path.join(tempfile.gettempdir(),
          "pert_" + os.path.basename(subm))

        tempFileSet.add(psubm)

        textout = os.path.join(tempfile.gettempdir(), psubm + ".out")

        # TODO
        # Custom parameters for each perturbation type? Use defaults for now
        command = (perturber, subm, psubm)

        #print "Running", command

        if perturber == "PassThrough":
          shutil.copyfile(subm, psubm)
        else:
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

        if debugAddRandom:
          metricValues += [random.uniform(0,100)]

        metricTable.append(metricValues)

      metricTable = np.array(metricTable, np.float64)

      # TODO: fill missing/nan values using transduction or draw from a
      # distribution estimate p(rank | submission)?

      print metricTable.shape
      print np.around(metricTable, decimals=5)

      rankagg = UnsupervisedLearningRankAggregator()
      rankagg.aggregate(metricTable, metricOrder)

      #averageWeights += rankagg.get_weights()
      w = rankagg.get_weights()

      #if t % 20 == 0:
      #  print "Weights", np.around(w, decimals=3)
      print "Weights", np.around(w, decimals=3)

      averageWeights += w

  #TODO?
  averageWeights /= numSubSamples
  #averageWeights /= numSamples

  averageWeights /= numPerturbations

  print "Sum weights = ", np.sum(averageWeights)

  print "Metric names:\n", metricNames
  print "Average weights =\n", np.around(averageWeights, decimals=3)

  imin = np.argmin(averageWeights)
  print "Smallest weight is", averageWeights[imin], "for", metricNames[imin]
  imax = np.argmax(averageWeights)
  print "Largest weight is", averageWeights[imax], "for", metricNames[imax]

  # Store average weights for future evaluation
  with open("metricNames.txt", 'w') as file:
    for item in metricNames:
      file.write("%s\n" % item)

  np.save("metric_weights.npy", averageWeights)

  # TODO
  # Apply computed weights to get rankings, averaged over different cases
  # finalRank = zeros(numSubmissions)
  # for gt in groundTruthFiles:
  #   rankagg = UnsupervisedLearningRankAggregator()
  #   rankagg.set_weights(averageWeights)
  #   rank_g = rankagg.get_aggregated_rank(metricTable, metricOrder)
  # finalRank /= numSubmissions
  # finalRank = np.argsort(finalRank)
  #print "Final aggregated rank\n", finalRank
  #print "Best submission is in", os.path.dirname(submissionFileTable[finalRank][0])


  # Clean up: remove perturbed images in temporary directory
  for f in tempFileSet:
    os.remove(f)
