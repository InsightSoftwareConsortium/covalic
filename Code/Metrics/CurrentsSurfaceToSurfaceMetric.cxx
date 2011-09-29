
#include "itkKdTree.h"
#include "itkKdTreeGenerator.h"
#include "itkListSample.h"
#include "itkVariableLengthVector.h"

#include "vtkPoints.h"
#include "vtkSmartPointer.h"
#include "vtkPolyData.h"
#include "vtkPolyDataReader.h"
#include "vtkPolyDataWriter.h"
#include "vtkTriangleFilter.h"

#include "vtkCellData.h"
#include "vtkDataArray.h"
#include "vtkFloatArray.h"
#include "vtkPointData.h"
#include "vtkSmartPointer.h"

#include "vnl/vnl_math.h"

#include "CurrentsSurfaceToSurfaceMetric.h"

#include <exception>
#include <stdexcept>

void
CurrentsSurfaceToSurfaceMetric
::SetKernelWidth(double d)
{
  m_KernelWidth = d;
}

double
CurrentsSurfaceToSurfaceMetric
::GetKernelWidth() const
{
  return m_KernelWidth;
}

double
CurrentsSurfaceToSurfaceMetric
::ComputeCurrentsNorm(vtkPolyData* polyData) const
{
  double var = m_KernelWidth * m_KernelWidth;

  double truncDist = 3.0*m_KernelWidth;

  vtkSmartPointer<vtkDataArray> normals = polyData->GetCellData()->GetNormals();

  // Build kd tree
  typedef itk::VariableLengthVector<float> VectorType;
  typedef itk::Statistics::ListSample<VectorType> SampleType;
  typedef itk::Statistics::KdTree<SampleType> KdTreeType;
  typedef itk::Statistics::KdTreeGenerator<SampleType> TreeGeneratorType;

  SampleType::Pointer kdsamples = SampleType::New();
  kdsamples->SetMeasurementVectorSize(3);
  kdsamples->Resize(polyData->GetNumberOfCells());

  for (vtkIdType i = 0; i < polyData->GetNumberOfCells(); i++)
  {
    VectorType c(3);
    c.Fill(0);

    vtkIdType nPts = 0;
    vtkIdType* ptIds = 0;
    polyData->GetCellPoints(i, nPts, ptIds);

    // Always assume triangles
    if (nPts != 3)
      throw std::runtime_error("Non triangle cell detected");

    for (int p = 0; p < 3; p++)
    {
      double x[3];
      polyData->GetPoint(ptIds[p], x);

      for (int dim = 0; dim < 3; dim++)
        c[dim] += x[dim];
    }

    c /= 3.0;

    kdsamples->SetMeasurementVector(i, c);
  }

  TreeGeneratorType::Pointer treeGenerator = TreeGeneratorType::New();

  treeGenerator->SetSample(kdsamples);
  treeGenerator->SetBucketSize(50);
  treeGenerator->Update();

  KdTreeType::Pointer kdtree = treeGenerator->GetOutput();

  // Evaluate with truncation radius = truncDist = 3*sigma
  double kdnorm = 0.0;

  for (unsigned int i = 0; i < polyData->GetNumberOfCells(); i++)
  {
    VectorType ci = kdsamples->GetMeasurementVector(i);

    double ni[3];
    normals->GetTuple(i, ni);

    KdTreeType::InstanceIdentifierVectorType neighbors;
    kdtree->Search(ci, truncDist, neighbors);

    for (unsigned int j = 0; j < neighbors.size(); j++)
    {
      VectorType dvec = kdsamples->GetMeasurementVector(neighbors[j]) - ci;

      double nj[3];
      normals->GetTuple(neighbors[j], nj);

      double dotnn = 0;
      for (unsigned int d = 0; d < 3; d++)
        dotnn += ni[d] * nj[d];

      //kdnorm += dot3d(ni, nj) * exp(-0.5 * dvec.GetSquaredNorm() / var);
      kdnorm += dotnn * exp(-0.5 * dvec.GetSquaredNorm() / var);
    }

  }
  kdnorm /=
    pow(2.0*var*vnl_math::pi, 3.0/2.0) + 1e-20;

  return kdnorm;

}

vtkSmartPointer<vtkPolyData> 
CurrentsSurfaceToSurfaceMetric
::ComputeWeightedPDNormals(vtkPolyData* polyData) const
{
  // Make sure we only have triangles (not strips or polys)
  vtkSmartPointer<vtkTriangleFilter> trif =
    vtkSmartPointer<vtkTriangleFilter>::New();
  trif->SetInput(polyData);
  trif->PassVertsOff();
  trif->PassLinesOff();
  trif->Update();

  vtkSmartPointer<vtkPolyData> triPD = trif->GetOutput();

  triPD->BuildLinks();

  // Compute cross product here
  vtkSmartPointer<vtkFloatArray> newNormals = vtkFloatArray::New();
  newNormals->SetNumberOfComponents(3);
  newNormals->SetNumberOfTuples(triPD->GetNumberOfCells());
  newNormals->SetName("Normals");

  for (vtkIdType i = 0; i < triPD->GetNumberOfCells(); i++)
  {
    vtkIdType nPts = 0;
    vtkIdType* ptIds = 0;
    triPD->GetCellPoints(i, nPts, ptIds);

    if (nPts != 3)
      throw std::runtime_error("Non triangle cell detected");

    double x0[3];
    triPD->GetPoint(ptIds[0], x0);
    double x1[3];
    triPD->GetPoint(ptIds[1], x1);
    double x2[3];
    triPD->GetPoint(ptIds[2], x2);

    for (int d = 0; d < 3; d++)
    {
      x1[d] = x1[d] - x0[d];
      x2[d] = x2[d] - x0[d];
    }

    //vtkScalar
    double wnormal[3];
    wnormal[0] = (x1[1] * x2[2] - x1[2] * x2[1]) / 2.0;
    wnormal[1] = (x1[2] * x2[0] - x1[0] * x2[2]) / 2.0;
    wnormal[2] = (x1[0] * x2[1] - x1[1] * x2[0]) / 2.0;

    newNormals->SetTuple(i, wnormal);
  }

  triPD->GetCellData()->SetNormals(newNormals);

  return triPD;
}


CurrentsSurfaceToSurfaceMetric::MeasureType
CurrentsSurfaceToSurfaceMetric
::GetValue() const
{
  // sum_i sum_j k(c_i, c_j) <n_i, n_j>

//TODO:
  // if (m_Approximation == ParticleMesh)
  //   return this->GetValueWithParticleMesh();

  // Compute weighted normals
  vtkSmartPointer<vtkPolyData> polyData1 =
    this->ComputeWeightedPDNormals(this->GetMovingSurface());
  vtkSmartPointer<vtkPolyData> polyData2 =
    this->ComputeWeightedPDNormals(this->GetFixedSurface());

  double var = m_KernelWidth * m_KernelWidth;

  double truncDist = 3.0*m_KernelWidth;

  //unsigned int minNeighborCount = polyData1->GetNumberOfCells() / 100 + 1;
  unsigned int minNeighborCount = 1;

  // Build kd tree
  typedef itk::VariableLengthVector<float> VectorType;
  typedef itk::Statistics::ListSample<VectorType> SampleType;
  typedef itk::Statistics::KdTree<SampleType> KdTreeType;
  typedef itk::Statistics::KdTreeGenerator<SampleType> TreeGeneratorType;

  SampleType::Pointer kdsamples = SampleType::New();
  kdsamples->SetMeasurementVectorSize(3);
  kdsamples->Resize(polyData1->GetNumberOfCells());

  // Compute triangle centroids
  for (vtkIdType i = 0; i < polyData1->GetNumberOfCells(); i++)
  {
    VectorType c(3);
    c.Fill(0);

    vtkIdType nPts = 0;
    vtkIdType* ptIds = 0;
    polyData1->GetCellPoints(i, nPts, ptIds);

    if (nPts != 3)
      throw std::runtime_error("Non triangle cell detected");

    for (int p = 0; p < 3; p++)
    {
      double x[3];
      polyData1->GetPoint(ptIds[p], x);

      for (int dim = 0; dim < 3; dim++)
        c[dim] += x[dim];
    }

    c /= 3.0;

    kdsamples->SetMeasurementVector(i, c);
  }

  TreeGeneratorType::Pointer treeGenerator = TreeGeneratorType::New();

  treeGenerator->SetSample(kdsamples);
  treeGenerator->SetBucketSize(50);
  treeGenerator->Update();

  KdTreeType::Pointer kdtree = treeGenerator->GetOutput();

  vtkSmartPointer<vtkDataArray> normals1 =
    polyData1->GetCellData()->GetNormals();
  vtkSmartPointer<vtkDataArray> normals2 =
    polyData2->GetCellData()->GetNormals();

  double match = 0;

  // first match term
  for (unsigned int i = 0; i < polyData1->GetNumberOfCells(); i++)
  {
    VectorType ci = kdsamples->GetMeasurementVector(i);

    double ni[3];
    normals1->GetTuple(i, ni);

    KdTreeType::InstanceIdentifierVectorType neighbors;
    kdtree->Search(ci, truncDist, neighbors);
    if (neighbors.size() < minNeighborCount)
      kdtree->Search(ci, minNeighborCount, neighbors);

    for (unsigned int j = 0; j < neighbors.size(); j++)
    {
      double nj[3];
      normals1->GetTuple(neighbors[j], nj);

      double dotnn = 0;
      for (unsigned int d = 0; d < 3; d++)
        dotnn += ni[d] * nj[d];

      VectorType dvec = ci - kdsamples->GetMeasurementVector(neighbors[j]);
      //double d = dot3d(ni, nj) * exp(-0.5 * dvec.GetSquaredNorm() / var);
      double d = dotnn * exp(-0.5 * dvec.GetSquaredNorm() / var);
      d /=  pow(2.0*var*vnl_math::pi, 3.0/2.0);

      match += d;
    }

  }

  // second match term
  for (vtkIdType i = 0; i < polyData2->GetNumberOfCells(); i++)
  {
    VectorType c_y(3);
    c_y.Fill(0);

    vtkIdType nPts = 0;
    vtkIdType* ptIds = 0;
    polyData2->GetCellPoints(i, nPts, ptIds);

    if (nPts != 3)
      throw std::runtime_error("Non triangle cell detected");

    for (int p = 0; p < 3; p++)
    {
      double y[3];
      polyData2->GetPoint(ptIds[p], y);

      for (int dim = 0; dim < 3; dim++)
        c_y[dim] += y[dim];
    }

    c_y /= 3.0;

    double n_y[3];
    normals2->GetTuple(i, n_y);

    KdTreeType::InstanceIdentifierVectorType neighbors;
    kdtree->Search(c_y, truncDist, neighbors);
    if (neighbors.size() < minNeighborCount)
      kdtree->Search(c_y, minNeighborCount, neighbors);

    for (unsigned int j = 0; j < neighbors.size(); j++)
    {
      double n_x[3];
      normals1->GetTuple(neighbors[j], n_x);

      double dotnn = 0;
      for (unsigned int d = 0; d < 3; d++)
        dotnn += n_x[d] * n_y[d];

      VectorType dvec = kdsamples->GetMeasurementVector(neighbors[j]) - c_y;
      //double d = dot3d(n_x, n_y) * exp(-0.5 * dvec.GetSquaredNorm() / var);
      double d = dotnn * exp(-0.5 * dvec.GetSquaredNorm() / var);
      d *= -2.0 / pow(2.0*var*vnl_math::pi, 3.0/2.0);

      match += d;
    }

  }

  match += this->ComputeCurrentsNorm(polyData2);

  return match;
}
