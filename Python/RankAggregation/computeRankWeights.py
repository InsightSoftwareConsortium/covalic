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
import time

import multiprocessing

import json
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
    description="Compute metric weights for ranking submissions in an image segmentation challenge")
  parser.add_argument("binaryPath", type=str,
    help="path containing perturbation and metric binaries")
  parser.add_argument("groundTruthRootPath", type=str,
    help="path containing folders with ground truth from different raters")
  parser.add_argument("submissionsRootPath", type=str,
    help="path containing folders for different submissions")
  parser.add_argument("weightsFile", type=str,
    help="file for storing competition weights in JSON format")
  parser.add_argument("-p", "--perturbations", type=int, default=100,
    help="number of perturbations per ground truth")
  parser.add_argument("-t", "--threads", type=int, default=8,
    help="number of threads (processes) for metric evaluations")

  args = parser.parse_args()

  binaryPath = args.binaryPath
  groundTruthRootPath = args.groundTruthRootPath
  submissionsRootPath = args.submissionsRootPath

  weightsFile = args.weightsFile

  numPerturbations = args.perturbations
  numProcesses = args.threads

  numCPU = multiprocessing.cpu_count()
  if numProcesses > numCPU or numProcesses < 1:
    print "Setting number of processes from", numProcesses, "to", numCPU
    numProcesses = numCPU

  # Limit number of ITK threads during metric evals and perturbations
  os.environ["ITK_GLOBAL_DEFAULT_NUMBER_OF_THREADS"] = str(8)

  print "Applying", numPerturbations, "perturbations per ground truth"
  print "Using", numProcesses, "processes"

  np.set_printoptions(precision=3)
  np.set_printoptions(formatter={'float': '{: 0.3f}'.format})
  np.set_printoptions(suppress=True)

  # Make multiple runs consistent
  random.seed(2015023468)
  np.random.seed(261310579)

  startTime = time.clock()

  metricBinaryList = sorted(glob.glob(os.path.join(binaryPath, "ValidateImage*")))

  # Remove Jaccard if we have both Dice and Jaccard (since they're equivalent)
  diceBinary = os.path.join(binaryPath, "ValidateImageDice")
  jaccardBinary = os.path.join(binaryPath, "ValidateImageJaccard")
  if diceBinary in metricBinaryList and jaccardBinary in metricBinaryList:
    metricBinaryList.remove(jaccardBinary)

  # DEBUG
  debugAddRandom = False

  if not metricBinaryList:
    print "No image metric apps detected"
    sys.exit(-1)

  numMetricBinaries = len(metricBinaryList)

  metricIDDict = dict()
  metricIDDict["ValidateImageAveDist"] = "Adb"
  metricIDDict["ValidateImageDice"] = "Dice"
  metricIDDict["ValidateImageHausdorffDist"] = "Hdb"
  metricIDDict["ValidateImageJaccard"] = "Jac"
  metricIDDict["ValidateImageKappa"] = "Kap"
  metricIDDict["ValidateImagePPV"] = "PPV"
  metricIDDict["ValidateImageSpecificity"] = "Spec"
  metricIDDict["ValidateImageSensitivity"] = "Sens"

  perturbBinaryList = glob.glob(os.path.join(binaryPath, "PerturbImageLabels*"))

  if not perturbBinaryList:
    print "No image perturbation apps detected"
    sys.exit(-1)

  numPerturbers = len(perturbBinaryList)

  # Parameters for different perturbation binaries
  perturbParameterDict = dict()
  perturbParameterDict["PerturbImageLabelsMorphology"] = \
    ["--iterations", "4", "--radius", "1"]
  perturbParameterDict["PerturbImageLabelsAffine"] = \
    ["--maxScaleFactor", "1.05", "--maxRotationAngle", "15", "--maxTranslation", "4" ]
  perturbParameterDict["PerturbImageLabelsBSpline"] = \
    ["--gridNodes", "8", "--normalVariance", "4.0"]

  # Get ground truth files
  groundTruthFilesTable = []
  for p in sorted(glob.glob(os.path.join(groundTruthRootPath, "*"))):
    if os.path.isdir(p):

      groundTruthFiles = \
        [f for f in sorted(glob.glob(os.path.join(p, "*"))) \
        if os.path.isfile(f)]

      groundTruthFilesTable.append(groundTruthFiles)

  # Find number of samples for ground truth, and the rater with the complete set
  numGroundTruthSamples = 0

  groundTruthCompleteFiles = []

  for files in groundTruthFilesTable:
    if len(files) > numGroundTruthSamples:
      numGroundTruthSamples = len(files)
      groundTruthCompleteFiles = files

  # Compute number of objects in ground truth set
  numObjects = 0
  for files in groundTruthFilesTable:
    for f in files:
      labelImage = sitk.ReadImage(f)
      labelArray = sitk.GetArrayFromImage(labelImage)
      maxLabel = np.max(labelArray)
      if maxLabel > numObjects:
        numObjects = maxLabel

  print "Analyzing", numObjects, "objects in all label images"

  # Order of metric for ranking (high is good = 1, high is bad = -1)
  # Distance measures are marked negative
  metricOrder = []
  for i in range(numMetricBinaries):
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
  for i in range(numMetricBinaries):
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
      for f in sorted(glob.glob(os.path.join(p, "*"))):
        if not os.path.isfile(f):
          continue

        q = getSubjectKey(f)

        match = False
        for t in groundTruthCompleteFiles:
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

    # Cycle between different raters
    groundTruthFiles = \
      groundTruthFilesTable[t % len(groundTruthFilesTable)]

    numGroundTruthSamples = len(groundTruthFiles)

    numGroundTruthSubSamples = int(numGroundTruthSamples * 0.6)

    # Pick a type of perturbation

    """
    i_pert = np.random.randint(0, numPerturbers+1)
    if i_pert < numPerturbers:
      perturber = perturbBinaryList[i_pert]
    else:
      perturber = "PassThrough"
    """

    i_pert = np.random.randint(0, numPerturbers)
    perturber = perturbBinaryList[i_pert]

    print "Applying", os.path.basename(perturber)

    # Evaluate random subset of ground truth cases, for robustness against
    # an outlier case
    gtIndices = np.random.permutation(numGroundTruthSamples)[:numGroundTruthSubSamples]

    def getMetricTable(gt):

      print "Examining ground truth:\n", gt

      pname = multiprocessing.current_process().name

      metricTable = []

      # Output perturbed image to temp dir
      pert_gt = os.path.join(tempfile.gettempdir(),
        pname + "_pert_" + os.path.basename(gt))

      tempFileSet.add(pert_gt)

      textout = os.path.join(tempfile.gettempdir(),
        pname + "_pert_" + os.path.basename(gt) + ".out")

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
        for j in range(numMetricBinaries):

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
      "with shape", metricTableList[0].shape

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

    print "Average weights after", t+1, "perturbations\n", averageWeights/(t+1)

  averageWeights /= numPerturbations

  print "--------------------"
  print "Final estimates"
  print "--------------------"

  print "Sum weights = ", np.sum(averageWeights)

  #print "Metric names:\n", metricNames
  #print "Average weights =\n", averageWeights

  print "Metric weights:"
  for i in range(len(metricNames)):
    print metricNames[i], "->", averageWeights[i]
  print "--------------------"

  #imin = np.argmin(averageWeights)
  #print "Smallest weight is", averageWeights[imin], "for", metricNames[imin]
  #imax = np.argmax(averageWeights)
  #print "Largest weight is", averageWeights[imax], "for", metricNames[imax]

  print "Sorted metric weights (largest to smallest):"
  isort = np.argsort(averageWeights * -1.0)
  for i in isort:
    print metricNames[i], "->", averageWeights[i]
  print "--------------------"

  print "Sum of weights by metric type:"
  metricTypeWeights = dict()
  for i in range(len(metricNames)):
    t = metricNames[i].split("_")[0]
    if not t in metricTypeWeights:
      metricTypeWeights[t] = averageWeights[i]
    else:
      metricTypeWeights[t] += averageWeights[i]
  for k in sorted(metricTypeWeights.keys()):
    print k, "->", metricTypeWeights[k]
  print "--------------------"

  print "Sum of weights by object:"
  objectWeights = dict()
  for i in range(len(metricNames)):
    t = metricNames[i].split("_")[1]
    if not t in objectWeights:
      objectWeights[t] = averageWeights[i]
    else:
      objectWeights[t] += averageWeights[i]
  for k in sorted(objectWeights.keys()):
    print k, "->", objectWeights[k]
  print "--------------------"

  # Store final weights for future evaluation

  """
  with open("metricNames.txt", 'w') as file:
    for item in metricNames:
      file.write("%s\n" % item)

  np.save("metric_weights.npy", averageWeights)
  """

  competitionWeights = dict()
  for i in range(len(metricNames)):
    name_obj = metricNames[i].split("_")
    name = name_obj[0]
    obj = name_obj[1][len('object'):]

    binaryName = "ValidateImage" + name

    if binaryName in metricIDDict:
      id = metricIDDict[binaryName] + obj
    else:
      id = name + obj

    if obj == "":
      obj = "all"

    # Assign ordered weights to each metric ID
    competitionWeights[id] = \
      { \
        "title":name + " (label " + obj +")", \
        "weight":(averageWeights[i] * metricOrder[i]) \
      }

  with open(weightsFile, "w") as outfile:
    json.dump(competitionWeights, outfile, indent=2, sort_keys=True)

  # Apply computed weights to get rankings, averaged over different ground
  # truth cases

  def getFinalRank(gt):

    print "Examining ground truth:\n", gt

    pname = multiprocessing.current_process().name

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

      textout = os.path.join(tempfile.gettempdir(),
        pname + "_" + os.path.basename(gt) + ".out")

      tempFileSet.add(textout)

      # Evaluate each metric on gt and perturbed submission
      metricValues = []
      for j in range(numMetricBinaries):

        # Run validation app and obtain list of metrics from stdout
        command = [metricBinaryList[j], gt, subm, textout]

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

    return rankagg.get_aggregated_rank(metricTable, metricOrder)

  pool = multiprocessing.Pool(processes=numProcesses)

  # TODO: compute final rank using multiple tables from multiple raters
  # TODO: move this to a separate script? applyRankWeights.py?
  finalRankTable = pool.map(getFinalRank, groundTruthCompleteFiles)

  pool.close()
  pool.join()

  finalAggregatedRank = np.zeros(numSubmissions)
  for rank_gt in finalRankTable:
    finalAggregatedRank += rank_gt

  finalAggregatedRank /= numGroundTruthSamples
  finalAggregatedRank = scipy.stats.rankdata(finalAggregatedRank)

  print "Final aggregated rank (over all ground truth)\n", finalAggregatedRank

  indSorted = np.argsort(finalAggregatedRank)

  print "Rankings:"
  for i in range(numSubmissions):
    i_s = indSorted[i]
    p = os.path.dirname(submissionFilesTable[i_s][0])
    print i+1, "->", os.path.basename(p)

  finalRankTable = np.array(finalRankTable)
  print "Final rank table (per ground truth)\n", finalRankTable

  # Remove perturbed images and metric evals in temporary directory
  print "Clean up"
  for f in tempFileSet:
    os.remove(f)

  print "Rank aggregation took", time.clock() - startTime, "seconds"
