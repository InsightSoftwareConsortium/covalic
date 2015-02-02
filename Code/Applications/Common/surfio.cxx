
#include "surfio.h"

#include "vtkCellArray.h"
#include "vtkOutputWindow.h"
#include "vtkPoints.h"
#include "vtkPolyData.h"
#include "vtkPolyDataReader.h"
#include "vtkPolyDataWriter.h"
#include "vtkSmartPointer.h"

#include "vtkBYUReader.h"
#include "vtkBYUWriter.h"

#include "vtkPLYReader.h"
#include "vtkPLYWriter.h"

#include <cstdlib>
#include <fstream>
#include <iostream>

static vtkSmartPointer< vtkPolyData > readOFF(const char* fn)
{
  vtkSmartPointer<vtkPolyData> pd = vtkSmartPointer<vtkPolyData>::New();
  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
  vtkSmartPointer<vtkCellArray> polys = vtkSmartPointer<vtkCellArray>::New();

  std::ifstream ifile;
  ifile.open(fn, ios::in);

  // Read first line (just a comment)
  char buf[256];
  ifile.getline(buf, 256);

  int n_pts = 0;
  int n_polys = 0;

  ifile >> n_pts;
  ifile >> n_polys;

  // Ignore remaining line
  ifile.getline(buf, 256);

  points->Allocate(n_pts);
  polys->Allocate(n_polys);

  // Read the points from the file
  double p[3];
  for(int i = 0; i < n_pts; i++)
  {
    ifile >> p[0];
    ifile >> p[1];
    ifile >> p[2];
    points->InsertPoint(i,p);
  }

  for(int i = 0; i < n_polys; i++)
  {
    vtkIdType n_vertices;
    ifile >> n_vertices;
    polys->InsertNextCell(n_vertices);
    vtkIdType vertex_id;
    for(int j = 0; j < n_vertices; j++)
    {
      ifile >> vertex_id;
      polys->InsertCellPoint(vertex_id);
    }
  }

  pd->SetPoints(points);
  pd->SetPolys(polys);

  ifile.close();

  return pd;
}

static vtkSmartPointer< vtkPolyData > readASC(const char* fn)
{
  vtkSmartPointer<vtkPolyData> pd = vtkSmartPointer<vtkPolyData>::New();
  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
  vtkSmartPointer<vtkCellArray> polys = vtkSmartPointer<vtkCellArray>::New();

  std::ifstream ifile;
  ifile.open(fn, ios::in);

  // Read first line (just a comment)
  char buf[256];
  ifile.getline(buf, 256);

  int n_pts = 0;
  int n_polys = 0;

  ifile >> n_pts;
  ifile >> n_polys;

  points->Allocate(n_pts);
  polys->Allocate(n_polys);

  // read the points from the file
  double p[3];
  for(int i = 0; i < n_pts; i++)
  {
    ifile >> p[0];
    ifile >> p[1];
    ifile >> p[2];
    points->InsertPoint(i, p);

    // Last entry is always zero?
    ifile.getline(buf, 256);
  }

  for(int i = 0; i < n_polys; i++)
  {
    //vtkIdType n_vertices;
    //fscanf(in,"%d ",&n_vertices);
    //polys->InsertNextCell(n_vertices);
    polys->InsertNextCell(3);
    vtkIdType vertex_id = 0;
    //for(int j = 0; j < n_vertices; j++)
    for(int j = 0; j < 3; j++)
    {
      ifile >> vertex_id;
      polys->InsertCellPoint(vertex_id);
    }

    // Last entry is always 0?
    ifile.getline(buf, 256);
  }

  pd->SetPoints(points);
  pd->SetPolys(polys);

  ifile.close();

  return pd;
}

vtkSmartPointer<vtkPolyData> readSurface(const char* fn)
{
  vtkSmartPointer<vtkPolyData> pd;

  if (std::string(fn).find(".asc") != std::string::npos)
  {
    pd = readASC(fn);
  }
  else if (std::string(fn).find(".byu") != std::string::npos)
  {
    vtkSmartPointer<vtkBYUReader> reader = vtkSmartPointer<vtkBYUReader>::New();
    reader->SetFileName(fn);
    reader->Update();
    pd = reader->GetOutput();
  }
  else if (std::string(fn).find(".off") != std::string::npos)
  {
    pd = readOFF(fn);
  }
  else if (std::string(fn).find(".vtk") != std::string::npos)
  {
    vtkSmartPointer<vtkPolyDataReader> reader = vtkSmartPointer<vtkPolyDataReader>::New();
    reader->SetFileName(fn);
    reader->Update();
    pd = reader->GetOutput();
  }
  else if (std::string(fn).find(".ply") != std::string::npos)
  {
    vtkSmartPointer<vtkPLYReader> reader = vtkSmartPointer<vtkPLYReader>::New();
    reader->SetFileName(fn);
    reader->Update();
    pd = reader->GetOutput();
  }
  else
  {
    std::cerr << "Unknown input extension" << std::endl;
    return 0;
  }

  return pd;
}

void writeSurface(const char* fn, vtkPolyData* pd)
{
  if (std::string(fn).find(".vtk") != std::string::npos)
  {
    vtkSmartPointer<vtkPolyDataWriter> writer =
      vtkSmartPointer<vtkPolyDataWriter>::New();
    //writer->SetFileTypeToBinary();
    writer->SetFileName(fn);
    writer->SetInputData(pd);
    writer->Update();
  }
  else if (std::string(fn).find(".byu") != std::string::npos)
  {
    vtkSmartPointer<vtkBYUWriter> writer =
      vtkSmartPointer<vtkBYUWriter>::New();
    //writer->SetFileTypeToBinary();
    writer->SetGeometryFileName(fn);
    writer->SetInputData(pd);
    writer->Update();
  }
  else if (std::string(fn).find(".ply") != std::string::npos)
  {
    vtkSmartPointer<vtkPLYWriter> writer =
      vtkSmartPointer<vtkPLYWriter>::New();
    //writer->SetFileTypeToBinary();
    writer->SetFileName(fn);
    writer->SetInputData(pd);
    writer->Update();
  }
  else
  {
    std::cerr << "Unknown output extension" << std::endl;
    throw "Bad ext";
  }
}
