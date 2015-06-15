COVALIC (Comparison and Validation for Image Computing)
==============

COVALIC provides the building blocks of a web-based platform for public
image analysis challenges.

This includes ITK-based C++ plugins for comparing different image processing
algorithms that was used in multiple public challenges as a backend server code.These plugins are ITK filter classes that are wrapped in a generic command line
interface described by XML files to facilitate batch processing
in a variety of systems.

There's also work in progress for a Python-based meta-analysis machine learning
framework that automatically determine weightings of different metrics, and
characterize how a challenge can have different results under specific
perturbations.

COVALIC is currently in use at the following sites:
http://challenge.kitware.com
https://www.virtualskeleton.ch
