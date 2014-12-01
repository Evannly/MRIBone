#ifndef UNIGRID_H
#define UNIGRID_H

#include <stdio.h>
#include "GeoCommon.h"
#include "LinkedList.h"
#include "TriangulationTree.h"
#include "Cube.h"
#include "PLYWriter.h"
#include <math.h>
#include <time.h>
#include "volume.h"
#include "curvature.h"
#include "HashMap.h"
#include <QProgressDialog>
#include <OpenMesh/Core/IO/MeshIO.hh>
#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>

typedef OpenMesh::TriMesh_ArrayKernelT<> TriMesh;

#define G_PI 3.1415926535897932385

struct UniTreeNode
{
	float min, max ;
	UniTreeNode** chd ;
};

class UniGrid
{
public:

	// Data
	int dim[ 3 ] ;
	float dis[ 3 ] ;
	float *offset[3] ;
	float *** data ;
	int d1, d2 ;
    float ratioxy,ratioz;
	float *verts;
	int totalverts ;
	int *tris;
	int totaltris ;

	// Octree structure for fast contouring
	UniTreeNode* root ;

	// Contouring tables
	TriangulationTree** ttrees ;
	Cube* cube ;

	// Timings
	clock_t meshTime, searchTime;

	/// Constructor: from an existing Volume object
	UniGrid( Volume* vol );


	/// Destructor
	~UniGrid() ;

	/// Initialization
	void init( );

	/// Contouring given a iso-value
    void genContour( float iso,QProgressDialog *progress);
    void genContour( float iso, int isConvex ,QProgressDialog *progress);
	inline void genContourCell( float iso, int isConvex, int i, int j, int k, LinkedList<Point3f>* vertlist, LinkedList<Triangle3i>* trianglist, HashMap* myhash, Status* status );
	inline float getData( int i, int j, int k ) ;

	/// Contouring acceleration
	void buildTree( ) ;
	void clearTree( UniTreeNode* node ) ;
	UniTreeNode* buildTree( int x, int y, int z, int len ) ;
	inline void genContourTree( UniTreeNode* node, int x, int y, int z, int len, float iso, int isConvex, LinkedList<Point3f>* vertlist, LinkedList<Triangle3i>* trianglist, HashMap* myhash, Status* status );

	
	/// Laplacian smoothing of the mesh	
	void smooth( int iters );

	/// Retrieve the contour
    void getMesh( int& numverts, int& numtris, float*& vertices, int*& triangles );
    void getMesh(TriMesh &mesh);
	/// Writing procedures
	void writePLY( char* fname );

	struct GridShift
	{
		float x;
		float y;
		float z;
	};

	GridShift m_GridShift;
};

#endif
