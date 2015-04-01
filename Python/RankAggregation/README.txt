
Rank aggregation using machine learning

===

TODO:

C++ binary / SimpleITK for perturbing segmentation
  deformation along normals of whole surface (non-zero mask)
  random dilation / erosion? 
    loop:
    flip coin -> dilation / erosion
    generate random structuring element with center = 1
    apply morpho filter, repeat

C++ binary / Python: ValidateImageRandom
  for any input return random(0, 100)

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
