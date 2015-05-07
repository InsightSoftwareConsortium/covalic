import os
import sys

import SimpleITK as sitk
import numpy as np

if __name__ == "__main__":

	if len(sys.argv) != 3:
		print "Usage:", sys.argv[0], " <image-to-perturb> <output-dir>"
		sys.exit(-1)
	
	# Read the image to perturb
	imageToPerturb = sitk.ReadImage(sys.argv[1])
		
	# Apply the erosion filter
	erodedImage = sitk.ErodeObjectMorphology(imageToPerturb, 1)

	# Output the perturbed image
	sitk.WriteImage(erodedImage, sys.argv[2])
