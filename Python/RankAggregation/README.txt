
Rank aggregation using machine learning

Requires: numpy, scipy, SimpleITK

Example usage:
python computeRankWeights.py /home/user/covalic-SuperBuild/Covalic-Build/lib/covalic/Plugins  /data/ground_truth_root /data/submissions_root

===

TODO:

C++ binary / SimpleITK for perturbing segmentation
  deformation along normals of whole surface (non-zero mask)

rankagg script:
  get binary path, ground truth path, submissions path
  determine list of binaries (ValidateImage*)
  compute metrics and rank (if binary has Distance in filename do -metric)
  add some bad metrics
    random
    volume of non-zero (A+B)
    Dice with half of image set to zero (incomplete): left of centroid(x) = 0

  algo:
  set weights 0
  compute weights for ranks
  loop
    (bagging? do subsets?)
    perturb segmentations, compute ranks
    compute weights'
    weight += weights'
  weight /= numPerturbations + 1

  assumes that weight near 0 for unreliable metrics (not consistent)

User input:
  priority mask for segmentations (specific case, boundary, center)
  weight for selection and ranking
  alter rank weighings
