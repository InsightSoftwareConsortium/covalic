
#include "CurrentsSurfaceToSurfaceMetric.h"
#include "ClosestDistanceSurfaceToSurfaceMetric.h"
#include "HaussdorffDistanceSurfaceToSurfaceMetric.h"

#include "itkOutputWindow.h"
#include "itkTextOutput.h"

#include "vtkSphereSource.h"

#include <exception>
#include <iostream>
#include <string>


int
testMetrics()
{

  itk::OutputWindow::SetInstance(itk::TextOutput::New());

  vtkSmartPointer<vtkPolyData> spherePD1;
  {
    vtkSmartPointer<vtkSphereSource> sphereS = vtkSmartPointer<vtkSphereSource>::New();
    //sphereS->SetCenter(4.0, 4.0, -2.5);
    //sphereS->SetCenter(0, -5, 5);
    //sphereS->SetRadius(10.0);
    sphereS->SetCenter(64, 64, 64);
    sphereS->SetRadius(32);
    sphereS->SetThetaResolution(20);
    sphereS->SetPhiResolution(20);
    sphereS->LatLongTessellationOff();
    sphereS->Update();
    spherePD1 = sphereS->GetOutput();
  }

  vtkSmartPointer<vtkPolyData> spherePD2;
  {
    vtkSmartPointer<vtkSphereSource> sphereS = vtkSmartPointer<vtkSphereSource>::New();
    //sphereS->SetCenter(4.0, 4.0, -2.5);
    //sphereS->SetCenter(0, -5, 5);
    //sphereS->SetRadius(10.0);
    sphereS->SetCenter(64, 64, 64);
    sphereS->SetRadius(48);
    sphereS->SetThetaResolution(25);
    sphereS->SetPhiResolution(25);
    sphereS->LatLongTessellationOff();
    sphereS->Update();
    spherePD2 = sphereS->GetOutput();
  }

  ClosestDistanceSurfaceToSurfaceMetric::Pointer closestMetric = ClosestDistanceSurfaceToSurfaceMetric::New();
  closestMetric->SetFixedSurface(spherePD1);
  closestMetric->SetMovingSurface(spherePD2);

  std::cout << "Closest(A,B) = " <<  closestMetric->GetValue() << std::endl;

  closestMetric->SetFixedSurface(spherePD1);
  closestMetric->SetMovingSurface(spherePD1);

  std::cout << "Closest(A,A) = " <<  closestMetric->GetValue() << std::endl;

  HaussdorffDistanceSurfaceToSurfaceMetric::Pointer haussdMetric = HaussdorffDistanceSurfaceToSurfaceMetric::New();
  haussdMetric->SetFixedSurface(spherePD1);
  haussdMetric->SetMovingSurface(spherePD2);

  std::cout << "Haussdorff(A,B) = " <<  haussdMetric->GetValue() << std::endl;

  haussdMetric->SetFixedSurface(spherePD1);
  haussdMetric->SetMovingSurface(spherePD1);

  std::cout << "Haussdorff(A,A) = " <<  haussdMetric->GetValue() << std::endl;

  CurrentsSurfaceToSurfaceMetric::Pointer currMetric = CurrentsSurfaceToSurfaceMetric::New();
  currMetric->SetFixedSurface(spherePD1);
  currMetric->SetMovingSurface(spherePD2);
  currMetric->SetKernelWidth(4.0);

  std::cout << "Currents(A,B) = " <<  currMetric->GetValue() << std::endl;

  currMetric->SetFixedSurface(spherePD1);
  currMetric->SetMovingSurface(spherePD1);
  currMetric->SetKernelWidth(4.0);

  std::cout << "Currents(A,A) = " <<  currMetric->GetValue() << std::endl;

  return 0;

}

int
main(int argc, char** argv)
{
  try
  {
    testMetrics();
  } 
  catch (itk::ExceptionObject& e)
  {
    std::cerr << e << std::endl;
    return -1;
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << std::endl;
    return -1;
  }
  catch (std::string& s)
  {
    std::cerr << "Exception: " << s << std::endl;
    return -1;
  }
  catch (...)
  {
    std::cerr << "Unknown exception" << std::endl;
    return -1;
  }

  return 0;


}
