
#ifndef _surfio_h

#include "vtkPolyData.h"
#include "vtkSmartPointer.h"

vtkSmartPointer< vtkPolyData > readSurface(const char* fn);

void writeSurface(const char* fn, vtkPolyData* surf);

#endif
