#ifndef MESHFIM2D_H
#define MESHFIM2D_H


#include "TriMesh.h"
#include "TriMesh_algo.h"

#ifdef WIN32
#include <Windows.h>
#endif


#include <typeinfo>
#include <functional>
#include <queue>
#include <list>
#include <set>

#ifndef _EPS
#define _EPS 1e-06
#endif

using namespace std;

enum LabelType { FarPoint = 0,ActivePoint, MaskPoint, SeedPoint,
  StopPoint, AlivePoint,ToBeAlivePoint };


class meshFIM {

  public:


    typedef int index;




    //  float Upwind(index vet);
    void MeshReader(char * filename);

    //  float LocalSolver(index C, TriMesh::Face triangle);

    void SetSeedPoint(std::vector<index> SeedPoints)
    {
      m_SeedPoints = SeedPoints;

      vector<index> nb;

      if (m_meshPtr)
      {
        m_meshPtr->InitializeAttributes(m_SeedPoints);
        /*if (!m_SeedPoints.empty())
          {
          int ns = m_SeedPoints.size();
          for (int s = 0; s < ns; s++)
          {

          nb = m_meshPtr->neighbors[m_SeedPoints[s]];
          for (int i = 0; i<nb.size();i++)
          {
          m_ActivePoints.push_back(nb[i]);
          }


          }


          }*/

      }
    }

    void SetMesh(TriMesh* mesh)
    {
      m_meshPtr = mesh;

      orient(m_meshPtr);//  Manasi

      // have to recompute the normals and other attributes required for rendering
      if (!m_meshPtr->normals.empty()) m_meshPtr->normals.clear();//  Manasi
      m_meshPtr->need_normals();//  Manasi
      if (!m_meshPtr->adjacentfaces.empty()) m_meshPtr->adjacentfaces.clear();//  Manasi
      m_meshPtr->need_adjacentfaces();//  Manasi
      if (!m_meshPtr->across_edge.empty()) m_meshPtr->across_edge.clear();//  Manasi
      m_meshPtr->need_across_edge();//  Manasi
      if (!m_meshPtr->tstrips.empty()) m_meshPtr->tstrips.clear();//  Manasi
      m_meshPtr->need_tstrips();//  Manasi

      // initialize mesh attributes: vertT, Face.T[], Face.speedInv
      if (/*m_meshPtr->*/SPEEDTYPE == CURVATURE)
      {
        m_meshPtr->need_curvatures();

      }
      if (/*m_meshPtr->*/SPEEDTYPE == NOISE)
      {
        m_meshPtr->need_noise();

      }
      m_meshPtr->need_speed();

      m_meshPtr->need_faceedges();
      if (m_SeedPoints.empty())
      {
        m_meshPtr->InitializeAttributes();
      }
      else
      {
        m_meshPtr->InitializeAttributes(m_SeedPoints);  // if seed points are given, treat them differently


      }




    }

    void InitializeLabels(int numBlock)
    {
      if (!m_meshPtr)
      {
        std::cout << "Label-vector size unknown, please set the mesh first..." << std::endl;
      }
      else
      {
        // initialize all labels to 'Far'
        int nv = m_meshPtr->vertices.size();
        if (m_VertLabel.size() != nv) m_VertLabel.resize(nv);
        if (m_BlockLabel.size() != numBlock) m_BlockLabel.resize(numBlock);

        for (int l = 0; l < nv; l++)
        {
          m_VertLabel[l] = FarPoint;
        }

        for (int l = 0; l < numBlock; l++)
        {
          m_BlockLabel[l] = FarPoint;
        }

        // if seeed-points are present, treat them differently
        if (!m_SeedPoints.empty())
        {
          for (int s = 0; s < m_SeedPoints.size(); s++)
          {
            m_BlockLabel[m_PartitionLabel[m_SeedPoints[s]]] = ActivePoint;//m_Label[s] = SeedPoint;
            m_VertLabel[m_SeedPoints[s]] =  SeedPoint;
            m_ActiveBlocks.insert(m_ActiveBlocks.end(), m_PartitionLabel[m_SeedPoints[s]]);
          }
        }
        else
          cout<< "Initialize seed points before labels!!!" << endl;

        //if (!m_SeedPoints.empty())
        //{
        //  int ns = m_SeedPoints.size();
        //  vector<index> nb;
        //  for (int s = 0; s < ns; s++)
        //  {

        //    nb = m_meshPtr->neighbors[m_SeedPoints[s]];
        //    for (int i = 0; i<nb.size();i++)
        //    {
        //      if (m_Label[nb[i]]!=SeedPoint)
        //      {
        //        m_ActivePoints.push_back(nb[i]);
        //      }
        //
        //    }


        //  }


        //}
      }
    }

    //void InitializeActivePoints()
    //{
    //  if (!m_SeedPoints.empty())
    //  {
    //    int ns = m_SeedPoints.size();
    //    vector<index> nb;
    //    for (int s = 0; s < ns; s++)
    //    {

    //      nb = m_meshPtr->neighbors[m_SeedPoints[s]];
    //      for (int i = 0; i<nb.size();i++)
    //      {
    //        if (m_Label[nb[i]]!=SeedPoint)
    //        {
    //          m_ActivePoints.push_back(nb[i]);
    //          m_Label[nb[i]] = ActivePoint;
    //        }

    //      }


    //    }


    //  }

    //}

    //float PointLength(point v)
    //{
    //  float length = 0;
    //  for (int i = 0; i<3;i++)
    //  {
    //    length += v[i]*v[i];
    //  }

    //  return sqrt(length);
    //
    //}

    void SetStopDistance(float d)
    {
      m_StopDistance = d;

    }


    void GenerateData(int numBlock);


    //void GraphPartition_Simple(int Kring, int numBlock);
    void GraphPartition_METIS(char* partfilename,int numBlock);
    void GraphPartition_METIS2(int& numBlock, int maxNumBlockVerts);
    void GraphPartition_Square(int squareLength,int squareWidth, int blockLength, int blockWidth);

    void GraphColoring();
    void PartitionFaces(int numBlock);





    meshFIM(){
      m_meshPtr = NULL;
    };
    ~meshFIM(){};




    TriMesh*                                     m_meshPtr;
    vector< set<int> >                           m_BlockNbPts;
    vector< set<int> >                           m_BlockNeighbor;
    vector<int>                                  m_BlockSizes;
    vector<int>                                  m_BlockPoints;
    vector<int>                                  m_ColorLabel;
    int                                          m_numColor;
    vector<Color>                                m_faceColors;
    vector< vector<int> >                        m_PartitionFaces;
    vector< vector<int> >                        m_PartitionVerts;
    vector< vector<int> >                        m_PartitionNbFaces;
    vector< vector<TriMesh::Face> >              m_PartitionVirtualFaces;
    int                                          m_maxNumTotalFaces;
    int                                          m_maxNumVert;
    int                                          m_maxNumVertMapping;
    std::vector<int>                                     m_PartitionLabel;    // label of vertices belong to which partition

  protected:

    std::set<index>                              m_ActiveBlocks;
    std::vector<index>                           m_SeedPoints;
    std::vector<LabelType>                       m_VertLabel;             // label of all the vertices active or not
    vector<LabelType>                            m_BlockLabel;            // label of blocks active or not


    float                                        m_StopDistance;



};


#endif
