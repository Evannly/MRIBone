#include "TriangulationTree.h"

TriangulationTree::TriangulationTree( char* fname, int len )
{
	this->length = len ;
	this->loadTable( fname ) ;
}

int TriangulationTree::triangulate( Point3D **vertList, LinkedList<Triangle3D> *&rvalue )
{
	int i = 0, ct = 0 ;

	while ( table[i].type == 0 )
	{
		i = table[i].branch[ eliminate( vertList, table[i].test ) ];
		ct ++ ;
	}

	rvalue = new LinkedList<Triangle3D>();

	for ( int j = 0 ; j < this->length - 2 ; j ++ )
	{
#ifdef INSIDE_CONVEX
		triangle.v1 = vertList[table[i].trilist[j][2]];
		triangle.v2 = vertList[table[i].trilist[j][1]];
		triangle.v3 = vertList[table[i].trilist[j][0]];
#else
		triangle.v1 = vertList[table[i].trilist[j][0]];
		triangle.v2 = vertList[table[i].trilist[j][1]];
		triangle.v3 = vertList[table[i].trilist[j][2]];
#endif
		rvalue->add( triangle ) ;

//		printf("[%d %d %d]",table[i].trilist[j][0], table[i].trilist[j][1], table[i].trilist[j][2] );
	}

	return ct ;
}

int TriangulationTree::triangulate( float vertlist[12][3], int vertind[12], LinkedList<Triangle3i> *&rvalue )
{
	int i = 0, ct = 0 ;

	while ( table[i].type == 0 )
	{
		i = table[i].branch[ eliminate( vertlist, table[i].test ) ];
		ct ++ ;
	}

	rvalue = new LinkedList<Triangle3i>();

	for ( int j = 0 ; j < this->length - 2 ; j ++ )
	{
#ifdef INSIDE_CONVEX
		trianglei.v1 = vertind[table[i].trilist[j][2]];
		trianglei.v2 = vertind[table[i].trilist[j][1]];
		trianglei.v3 = vertind[table[i].trilist[j][0]];
#else
		trianglei.v1 = vertind[table[i].trilist[j][0]];
		trianglei.v2 = vertind[table[i].trilist[j][1]];
		trianglei.v3 = vertind[table[i].trilist[j][2]];
#endif
		rvalue->add( trianglei ) ;

//		printf("[%d %d %d]",table[i].trilist[j][0], table[i].trilist[j][1], table[i].trilist[j][2] );
	}

	return ct ;
}

int TriangulationTree::eliminate( Point3D **vertList, int quad[4] )
{
	COORDTYPE v[4][3];
	COORDTYPE cr[3];
	COORDTYPE f;

	// Form vectors
	v[0][0] = vertList[quad[2]]->x - vertList[quad[1]]->x ;
	v[1][0] = vertList[quad[3]]->x - vertList[quad[2]]->x ;
	v[2][0] = vertList[quad[0]]->x - vertList[quad[1]]->x ;

	v[0][1] = vertList[quad[2]]->y - vertList[quad[1]]->y ;
	v[1][1] = vertList[quad[3]]->y - vertList[quad[2]]->y ;
	v[2][1] = vertList[quad[0]]->y - vertList[quad[1]]->y ;

	v[0][2] = vertList[quad[2]]->z - vertList[quad[1]]->z ;
	v[1][2] = vertList[quad[3]]->z - vertList[quad[2]]->z ;
	v[2][2] = vertList[quad[0]]->z - vertList[quad[1]]->z ;
	// Calculate cross product
	cr[0] = v[0][1] * v[1][2] - v[0][2] * v[1][1] ;
	cr[1] = v[0][2] * v[1][0] - v[0][0] * v[1][2] ;
	cr[2] = v[0][0] * v[1][1] - v[0][1] * v[1][0] ;

	// Calculate dot product
	f = v[2][0] * cr[0] + v[2][1] * cr[1] + v[2][2] * cr[2] ;
	
	if ( f > 0.0000001 )
	{
		return 0;
	}
	else // if ( f < - 0.0000001 )
	{
		return 1;
	}
};	

int TriangulationTree::eliminate( float vertList[12][3], int quad[4] )
{
	COORDTYPE v[4][3];
	COORDTYPE cr[3];
	COORDTYPE f;

	// Form vectors
	v[0][0] = vertList[quad[2]][0] - vertList[quad[1]][0] ;
	v[1][0] = vertList[quad[3]][0] - vertList[quad[2]][0] ;
	v[2][0] = vertList[quad[0]][0] - vertList[quad[1]][0] ;

	v[0][1] = vertList[quad[2]][1] - vertList[quad[1]][1] ;
	v[1][1] = vertList[quad[3]][1] - vertList[quad[2]][1] ;
	v[2][1] = vertList[quad[0]][1] - vertList[quad[1]][1] ;

	v[0][2] = vertList[quad[2]][2] - vertList[quad[1]][2] ;
	v[1][2] = vertList[quad[3]][2] - vertList[quad[2]][2] ;
	v[2][2] = vertList[quad[0]][2] - vertList[quad[1]][2] ;
	// Calculate cross product
	cr[0] = v[0][1] * v[1][2] - v[0][2] * v[1][1] ;
	cr[1] = v[0][2] * v[1][0] - v[0][0] * v[1][2] ;
	cr[2] = v[0][0] * v[1][1] - v[0][1] * v[1][0] ;

	// Calculate dot product
	f = v[2][0] * cr[0] + v[2][1] * cr[1] + v[2][2] * cr[2] ;
	
	if ( f > 0.0000001 )
	{
		return 0;
	}
	else // if ( f < - 0.0000001 )
	{
		return 1;
	}
};	

TriangulationTree::~TriangulationTree( )
{
	delete this->table ;
}

void TriangulationTree::loadTable( char* fname )
{
	FILE *fin;
	if ( ( fin = fopen( fname, "r" ) ) ==  NULL )
	{
		printf("Triangulation table %s not found!\n", fname);
		exit(0);
	}

	// Read in table size
	fscanf( fin, "%d\n", &(this->tablesize));
	this->table = new TriangulationTreeNode[ this->tablesize ] ;

	// Read in table entries
	char line[200];
	int i, j, k;
	char *token;
	for ( i = 0 ; i < this->tablesize ; i ++ )
	{
		fgets( line, 200, fin ) ;
		

		if ( line[2] != '{' )
		{
			// An intermediate node
			this->table[i].type = 0 ;

			// Parse the test list
			token = strtok( line, "{ ,\t\n" );
			sscanf( token, "%d", &(table[i].test[0]) );
			table[i].test[0] -- ;
			for ( j = 1 ; j < 4 ; j ++ )
			{
				token = strtok( NULL, "{ ,\t\n" );
				sscanf( token, "%d", &(table[i].test[j]) );
				table[i].test[j] -- ;
			}

			// Parse the branching indices
			for ( j = 0 ; j < 2 ; j ++ )
			{
				token = strtok( NULL, "{ ,\t\n" );
				sscanf( token, "%d", &(table[i].branch[j]) );
				table[i].branch[j] -- ;
			}
		}
		else
		{
			// A leaf node
			this->table[i].type = 1 ;
			this->table[i].trilist = new int* [this->length - 2];
			for ( j = 0 ; j < this->length - 2 ; j ++ )
			{
				table[i].trilist[j] = new int[3] ;
			}

			// Parse the triangle list
			int isFirst = 1;
			for ( j = 0 ; j < this->length - 2 ; j ++ )
			{
				for ( k = 0 ; k < 3 ; k ++ )
				{
					if ( isFirst )
					{
						token = strtok( line, "{ ,\t\n" );
						isFirst = 0 ;
					}
					else
					{
						token = strtok( NULL, "{ ,\t\n" );
					}
					sscanf( token, "%d", &(table[i].trilist[j][k]) );
					table[i].trilist[j][k] -- ;
				}
			}
		}
	
	}

	printf("Triangulation table constructed for length %d\n", this->length );
	fclose( fin );
}

