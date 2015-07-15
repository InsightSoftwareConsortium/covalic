
Rank aggregation using machine learning
===

Requires: numpy, scipy, SimpleITK

Scripts and classes for aggregating rankings from a variety of image processing
comparison metrics.

The primary script is computeRankWeights.py which generates a JSON file
containing the metric weights, given ground truth from multiple raters and 
submissions from different users. For example, given ground truth stored
in /data/ground_truth_root and submissions in /data/submissions_root, the
script can be invoked as follows:
```
    python computeRankWeights.py /home/user/covalic-SuperBuild/Covalic-Build/lib/covalic/Plugins /data/ground_truth_root /data/submissions_root weights.json
```

Right now, the script assumes that the file structure exist in the following
pattern:
```
    /data/ground_truth_root
      +- Rater1/
         +- case001_rater1.mha
         +- case002_rater1.mha
      +- Rater2/
         +- case001_rater2.mha
         +- case002_rater2.mha
    /data/submissions_root
      +- Submission1/
         +- case001_submitter01.mha
         +- case002_submitter01.mha
      +- Submission2/
         +- case001_submitter02.mha
         +- case002_submitter02.mha
      +- Submission3/
         +- case002_submitter03.mha
```
Missing submissions are allowed, however this will penalize the ranking of
the submitted results.

To-Do List
---

User input for learning to rank
- priority mask for segmentations (specific case, boundary, center)
- weight for selection and ranking
- alter rank weighings
