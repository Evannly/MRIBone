#ifndef GEOCOMMON_H
#define GEOCOMMON_H

//#define INSIDE_CONVEX

const int vertmap[12][2] = {{0,4},{1,5},{2,6},{3,7},{0,2},{1,3},{4,6},{5,7},{0,1},{2,3},{4,5},{6,7}};
const int coordmap[8][3] = {{0,0,0},{0,0,1},{0,1,0},{0,1,1},{1,0,0},{1,0,1},{1,1,0},{1,1,1}};
const int dirmap[12] = {0,0,0,0,1,1,1,1,2,2,2,2};
const int dirmap1[12] = {1,1,1,1,2,2,2,2,0,0,0,0};
const int dirmap2[12] = {2,2,2,2,0,0,0,0,1,1,1,1};
const int dismap1[12] = {0,0,1,1,0,1,0,1,0,0,1,1};
const int dismap2[12] = {0,1,0,1,0,0,1,1,0,1,0,1};

// 3d point with float coordinates and float normal
struct Vertex3D
{
	float coord[3];
	float nx, ny, nz;
};

// 3d point with float coordinates
struct Point3D
{
	float x, y, z;
	float nx, ny, nz ;
};
struct Point3f
{
	float x, y, z;
};

// 3d point with integer coordinates
struct Point3i
{
	int x, y, z;
};

// lower left and upper right cornes of a box
struct GridBox
{
	Point3i begin;
	Point3i end;
} ;

// Bounding box
struct BoundingBox
{
	Point3D begin;
	Point3D end;
};

// triangle that points to three vertices
struct Triangle3D
{
	Point3D *v1, *v2, *v3;
};
struct Triangle3i
{
	int v1, v2, v3;
};

// Half plane reprensentation
struct HalfPlane
{
	Point3D *base;
	float nx, ny, nz;
};


struct Status
{
	int num_cycles;
	int num_cells ;
	int total_cyclen;
	int total_tris ;
	int total_tests;
};


#endif
