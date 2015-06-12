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
import scipy.stats

import os, sys
import glob

import random

import multiprocessing

import shutil
import subprocess
import tempfile

def getMetricValues(textFilename, numLabels):
  """
  Get list of metric values from a text file, accounting for missing objects.
  """

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

  import argparse

  parser = argparse.ArgumentParser(
    description="Compute rank weights in an image segmentation challenge")
  parser.add_argument("binaryPath", type=str,
    help="path containing perturbation and metric binaries")
  parser.add_argument("groundTruthPath", type=str,
    help="path containing ground truth files")
  parser.add_argument("submissionsRootPath", type=str,
    help="path containing folders for different submissions")
  parser.add_argument("-p", "--perturbations", type=int, default=100,
    help="number of perturbations per ground truth")
  parser.add_argument("-t", "--threads", type=int, default=8,
    help="number of threads (processes) for metric evaluations")

  args = parser.parse_args()

  binaryPath = args.binaryPath
  groundTruthPath = args.groundTruthPath
  submissionsRootPath = args.submissionsRootPath

  numPerturbations = args.perturbations
  numProcesses = args.threads

  numCPU = multiprocessing.cpu_count()
  if numProcesses > numCPU or numProcesses < 1:
    print "Setting number of processes from", numProcesses, "to", numCPU
    numProcesses = numCPU

  print "Applying", numPerturbations, "perturbations per ground truth"
  print "Using", numProcesses, "processes"

  np.set_printoptions(precision=3)
  np.set_printoptions(formatter={'float': '{: 0.3f}'.format})
  np.set_printoptions(suppress=True)

  # Make multiple runs consistent
  random.seed(2790150234680)
  np.random.seed(78261310579)

  metricBinaryList = sorted(glob.glob(os.path.join(binaryPath, "ValidateImage*")))

  # DEBUG
  debugAddRandom = False

  if not metricBinaryList:
    print "No image metric apps detected"
    sys.exit(-1)

  numMetrics = len(metricBinaryList)

  perturbBinaryList = glob.glob(os.path.join(binaryPath, "PerturbImageLabels*"))

  if not perturbBinaryList:
    print "No image perturbation apps detected"
    sys.exit(-1)

  numPerturbers = len(perturbBinaryList)

  # Parameters for different perturbation binaries
  perturbParameterDict = dict()
  perturbParameterDict["PerturbImageLabelsMorphology"] = \
    ["--iterations", "2", "--radius", "1"]
  perturbParameterDict["PerturbImageLabelsAffine"] = \
    ["--maxScaleFactor", "1.02", "--maxRotationAngle", "4", "--maxTranslation", "2" ]
  perturbParameterDict["PerturbImageLabelsBSpline"] = \
    ["--gridNodes", "8", "--normalVariance", "1.5"]

  # Get ground truth files
  groundTruthFiles = \
    [f for f in sorted(glob.glob(os.path.join(groundTruthPath, "*"))) \
     if os.path.isfile(f)]

  numGroundTruthSamples = len(groundTruthFiles)

  numGroundTruthSubSamples = int(numGroundTruthSamples * 0.8)

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

  metricOrder = np.array(metricOrder)

  # Name of each element in evaluation (by metric-object)
  metricNames = []
  for i in range(numMetrics):
    m = os.path.basename(metricBinaryList[i])
    m = m[len("ValidateImage"):]
    if m.find("Kappa") >= 0:
      metricNames += [m + "_all"]
    else:
      for j in range(numObjects):
        metricNames += [m + "_object" + str(j+1)]

  if debugAddRandom:
    metricNames += ["Random"]

  print "Metric names:\n", metricNames
  print "Metric order:\n", metricOrder

  # Get the submission files
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

  numSubmissions = len(submissionFilesTable)

  #
  # Compute average weights with different perturbations and bagging
  # Each entry represents multiple metrics and multiple objects
  #

  averageWeights = np.zeros(len(metricNames))

  tempFileSet = set() # Hash of temporary files to delete later

  for t in range(numPerturbations):

    # Pick a type of perturbation
    i_pert = np.random.randint(0, numPerturbers+1)
    if i_pert < numPerturbers:
      perturber = perturbBinaryList[i_pert]
    else:
      perturber = "PassThrough"

    print "Applying", os.path.basename(perturber)

    # Evaluate random subset of ground truth cases, for robustness against
    # an outlier case
    gtIndices = np.random.permutation(numGroundTruthSamples)[:numGroundTruthSubSamples]

    def getMetricTable(gt):
      print "Examining ground truth:", os.path.basename(gt)

      metricTable = []

      # Output perturbed image to temp dir
      pert_gt = os.path.join(tempfile.gettempdir(),
        "pert_" + os.path.basename(gt))

      tempFileSet.add(pert_gt)

      textout = os.path.join(tempfile.gettempdir(),
        "pert_" + os.path.basename(gt) + ".out")

      tempFileSet.add(textout)

      command = [perturber, gt, pert_gt]

      # Use custom parameters for each perturbation type
      p_key = os.path.basename(perturber)
      if p_key in perturbParameterDict:
        command += perturbParameterDict[p_key]

      #print "Running", command

      if perturber == "PassThrough":
        shutil.copyfile(gt, pert_gt)
      else:
        p = subprocess.Popen(args=command, stdout=subprocess.PIPE,
          stderr=subprocess.PIPE)
        stdout, stderr = p.communicate()

      for submissionFiles in submissionFilesTable:

        # Search for match to this ground truth in submissions list
        subm = None
        for f in submissionFiles:
          q = getSubjectKey(f)

          if os.path.basename(gt).find(q) >= 0:
            subm = f

        # Handle missing submissions
        if subm is None:
          print "WARNING: Cannot find any submissions for", gt, \
            "in path", os.path.dirname(submissionFiles[0])
          metricTable.append( [np.nan] * len(metricNames) )
          continue

        # Evaluate each metric on gt and perturbed submission
        metricValues = []
        for j in range(numMetrics):

          # Run validation app and obtain list of metrics from stdout
          command = [metricBinaryList[j], pert_gt, subm, textout]

          #print "Running", command

          p = subprocess.Popen(args=command, stdout=subprocess.PIPE,
            stderr=subprocess.PIPE)
          stdout, stderr = p.communicate()

          metricValues += getMetricValues(textout, numObjects)

        if debugAddRandom:
          metricValues += [random.uniform(0,100)]

        metricTable.append(metricValues)

      metricTable = np.array(metricTable)

      # TODO: fill missing/nan values using transduction or draw from a
      # distribution estimate p(rank | submission)? uniform in [min, max]?

      return metricTable

    pool = multiprocessing.Pool(processes=numProcesses)

    metricTableList = pool.map(getMetricTable,
      [groundTruthFiles[i_gt] for i_gt in gtIndices])

    pool.close()
    pool.join()

    print "Number of metric tables", len(metricTableList), \
      "with shape", metricTable[0].shape

    for metricTable in metricTableList:
      print metricTable

    print "Aggregating rankings from metric tables"

    rankagg = UnsupervisedLearningRankAggregator()
    rankagg.aggregate(metricTableList, metricOrder)

    #averageWeights += rankagg.get_weights()
    w = rankagg.get_weights()

    #if t % 20 == 0:
    #  print "Weights", w
    print "Weights\n", w

    averageWeights += w

    print "Average weights\n", averageWeights/(t+1)

  averageWeights /= numPerturbations

  print "--------------------"
  print "Final estimates"
  print "--------------------"

  print "Sum weights = ", np.sum(averageWeights)

  print "Metric names:\n", metricNames
  print "Average weights =\n", averageWeights

  for i in range(len(metricNames)):
    print metricNames[i], "->", averageWeights[i]

  imin = np.argmin(averageWeights)
  print "Smallest weight is", averageWeights[imin], "for", metricNames[imin]
  imax = np.argmax(averageWeights)
  print "Largest weight is", averageWeights[imax], "for", metricNames[imax]

  # Store average weights for future evaluation
  with open("metricNames.txt", 'w') as file:
    for item in metricNames:
      file.write("%s\n" % item)

  np.save("metric_weights.npy", averageWeights)

  # Apply computed weights to get rankings, averaged over different ground
  # truth cases
  finalAggregatedRank = np.zeros(numSubmissions)

  finalRankTable = []

  for gt in groundTruthFiles:
    metricTable = []
    for submissionFiles in submissionFilesTable:

      # Search for match to this ground truth in submissions list
      subm = None
      for f in submissionFiles:
        q = getSubjectKey(f)

        if os.path.basename(gt).find(q) >= 0:
          subm = f

      # Handle missing submissions
      if subm is None:
        print "WARNING: Cannot find any submissions for", gt, \
          "in path", os.path.dirname(submissionFiles[0])
        metricTable.append( [np.nan] * len(metricNames) )
        continue

      # Evaluate each metric on gt and perturbed submission
      metricValues = []
      for j in range(numMetrics):

        # Run validation app and obtain list of metrics from stdout
        command = [metricBinaryList[j], pert_gt, subm, textout]

        #print "Running", command

        p = subprocess.Popen(args=command, stdout=subprocess.PIPE,
          stderr=subprocess.PIPE)
        stdout, stderr = p.communicate()

        metricValues += getMetricValues(textout, numObjects)

        if debugAddRandom:
          metricValues += [random.uniform(0,100)]

      metricTable.append(metricValues)

    metricTable = np.array(metricTable)

    rankagg = UnsupervisedLearningRankAggregator()
    rankagg.set_weights(averageWeights)

    rank_gt = rankagg.get_aggregated_rank(metricTable, metricOrder)

    finalRankTable.append(rank_gt)

    finalAggregatedRank += rank_gt

  finalAggregatedRank /= numGroundTruthSamples
  finalAggregatedRank = scipy.stats.rankdata(finalAggregatedRank) - 1

  print "Final aggregated rank (over all ground truth)\n", finalAggregatedRank

  indSorted = np.argsort(finalAggregatedRank)

  print "Rankings:"
  for i in numSubmissions:
    print i+1, "->", os.path.basename(os.path.dirname(submissionFilesTable[indSorted[i]][0]))

  finalRankTable = np.array(finalRankTable)
  print "Final rank table (per ground truth)\n", finalRankTable

  # Remove perturbed images and metric evals in temporary directory
  print "Clean up"
  for f in tempFileSet:
    os.remove(f)
