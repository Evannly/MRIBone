#ifndef TRIANGULATIONTREE_H
#define TRIANGULATIONTREE_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "LinkedList.h"
#include "GeoCommon.h"

#define COORDTYPE float


/* Entry in the triangulation table */
struct TriangulationTreeNode
{
	/// Node type: 0 for internal node, 1 for leaf node
	int type;

	/// For internal node, quad test vertices and branching indices
	int test[4];
	int branch[2];

	/// For leaf node, list of triangles 
	int** trilist;
};

/* Class for triangulator using diagonal-elimination tree */
class TriangulationTree 
{
public:

	/// Length of polygon in question
	int length;

	/// Size of table
	int tablesize;

	/// Triangulation table
	TriangulationTreeNode* table;

	/// Constructor
	TriangulationTree ( char* fname, int len );

	/// Destructor
	~TriangulationTree( ) ;

	/// Triangulate a given polygon, returning the number of tests
	int triangulate ( Point3D **vertList, LinkedList<Triangle3D> *&rvalue );
	int triangulate ( float vertList[12][3], int vertind[12], LinkedList<Triangle3i> *&rvalue );

protected:
	Triangle3D triangle;
	Triangle3i trianglei;

	/// Parser of table file
	void loadTable ( char* fname );

	/// Diagonal elimination 
	int  eliminate ( Point3D **vertList, int quad[4] );
	int  eliminate ( float vertList[12][3], int quad[4] );

};

















#endif