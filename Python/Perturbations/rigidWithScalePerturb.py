import os
import sys
import random

import SimpleITK as sitk
import numpy as np

if __name__ == "__main__":

	if len(sys.argv) != 3:
		print "Usage:", sys.argv[0], " <image-to-perturb> <output-file>"
		sys.exit(-1)
	
	# Read the image to perturb
	imageToPerturb = sitk.ReadImage(sys.argv[1])

	# Generate three random angles
	degsToRads = (3.14/180.0)
	angX = random.normalvariate(0, 5)*degsToRads
	angY = random.normalvariate(0, 5)*degsToRads
	angZ = random.normalvariate(0, 5)*degsToRads

	# Generate 3 random translations
	transX = random.normalvariate(0, 3)
	transY = random.normalvariate(0, 3)
	transZ = random.normalvariate(0, 3)

	# The rigid transformation
	rigidTransform = sitk.Transform(3, sitk.sitkEuler)
	rigidTransform.SetParameters([angX, angY, angZ, transX, transY, transZ]) 

	# Apply the rigid transformation
	imageRigid = sitk.Resample(imageToPerturb, imageToPerturb, rigidTransform, sitk.sitkNearestNeighbor, 0, sitk.sitkFloat32)

	# Random scaling
	scaleX = random.normalvariate(1, 0.1)
	scaleY = random.normalvariate(1, 0.1)
	scaleZ = random.normalvariate(1, 0.1)

	# The scaling transformation
	scaleTransform = sitk.Transform(3, sitk.sitkScale)
	scaleTransform.SetParameters([scaleX, scaleY, scaleZ])

	# Apply the scaling transformation
	imageScale = sitk.Resample(imageRigid, imageRigid, scaleTransform, sitk.sitkNearestNeighbor, 0, sitk.sitkFloat32)

	# Output the perturbed image
	sitk.WriteImage(imageScale, sys.argv[2])
