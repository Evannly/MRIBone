#include "Cube.h"
#include "LinkedList.h"
#include <stdlib.h>
// #include "Triangulator.h"
// #include <time.h>



/** BEGIN PARSING ROUTINES **/

char getTheEdge ( int v1, int v2 )
{
	for ( int i = 0 ; i < 12 ; i ++ )
	{
		if ( vertmap[i][0] == v1 && vertmap[i][1] == v2 )
		{
			return (char)i;
		}
	}
	printf("Reading error.... %d %d\n", v1, v2 );
	exit(0);

}

char getChar ( FILE *fptr, int &counter )
{
	char rvalue;

	do 
	{
		fscanf ( fptr, "%c", &rvalue );
		counter++;
	} while ( rvalue == ' ' || rvalue == '\r' || rvalue == '\n' );

	// printf("Char read: %c\n", rvalue);
	return rvalue;
}

// WARNING: consumes the character after the number as well!!!!
float getFloat ( FILE *fptr, int &counter ) 
{
	char number [ 256 ];
	int end = 0;
	char temp;

	temp = getChar ( fptr, counter );
	while ( ( ( temp >= '0' && temp <= '9' ) || temp == '.' || temp == '-' ) && end < 256 )
	{
		number [ end++ ] = temp;

		temp = getChar ( fptr, counter );
	}

	number [ end ] = '\0';

	return (float)atof ( number );
}

// WARNING: consumes the character after the number as well!!!!
int getInt ( FILE *fptr, int &counter ) 
{
	char number [ 256 ];
	int end = 0;
	char temp;

	temp = getChar ( fptr, counter );
	while ( ( temp >= '0' && temp <= '9' ) && end < 256 )
	{
		number [ end++ ] = temp;

		temp = getChar ( fptr, counter );
	}

	number [ end ] = '\0';

	return atoi ( number );
}

/*
Point3D *getArray ( FILE *fptr, int &counter )
{
	LinkedList<Point3D> pointList;
	Point3D point;
	char temp;

	temp = getChar ( fptr, counter );
	if ( temp != '{' ) 
	{
		fprintf ( stderr, "ERROR: %c found where { was expected\n", temp );
		return NULL;
	}

	temp = getChar ( fptr, counter );
	while ( temp == '{' ) 
	{
		// another point started
		point.x = getFloat ( fptr, counter );
		point.y = getFloat ( fptr, counter );
		point.z = getFloat ( fptr, counter );

		pointList.add ( point );

		temp = getChar ( fptr, counter );
		if ( temp == ',' )
		{
			temp = getChar ( fptr, counter );
		}
	}


	if ( temp != '}' ) 
	{
		fprintf ( stderr, "ERROR: %c found where } was expected\n", temp );
		return NULL;
	}

	// throw away the first one because it's junk
	return pointList.toArray ( );
}
*/

Cycle* getCycle ( FILE *fptr, int &counter ) 
{
	LinkedList<char> cycleList;
	char pair;
	char temp;

	temp = getChar ( fptr, counter );
	if ( temp != '{' ) 
	{
		fprintf ( stderr, "ERROR: %c found where { was expected\n", temp );
		return NULL;
	}

	temp = getChar ( fptr, counter );
	while ( temp == '{' ) 
	{
		// gotta subtract 1 because Mathematica starts indexing at 1, but we start at 0
		int ed1 = getInt ( fptr, counter ) - 1 ;
		int ed2 = getInt ( fptr, counter ) - 1 ;
		pair = getTheEdge( ed1, ed2 );

		cycleList.add ( pair );

		temp = getChar ( fptr, counter );
		if ( temp == ',' )
		{
			temp = getChar ( fptr, counter );
		}
	}

	if ( temp != '}' ) 
	{
		fprintf ( stderr, "ERROR: %c found where } was expected\n", temp );
		return NULL;
	}

	return new Cycle ( &cycleList );
}

LinkedList<Cycle*>** getTable ( FILE *fptr, int &counter ) 
{
	LinkedList<Cycle*> *cycleList;
	LinkedList<Cycle*>** rvalue = new LinkedList<Cycle*>* [ 256 ];
	char temp;
	int i;
	int maxCycle = 0;
	int sumCycle = 0;
	int numCycle = 0;
	int longCycle = 0;


	for ( i = 0; i < 256; i++ ) 
	{
		// printf("%d\n",i);
		cycleList = new LinkedList<Cycle*> ( );

		temp = getChar ( fptr, counter );
		if ( temp != '{' ) 
		{
			fprintf ( stderr, "ERROR: %c found where { was expected\n", temp );
			return NULL;
		}

		int offset = ftell ( fptr );
		int junk = 0;
		temp = getChar ( fptr, junk );
		if ( temp != '}' ) 
		{
			fseek ( fptr, offset, SEEK_SET );

			temp = ',';

			while ( temp == ',' ) 
			{
				Cycle *cycle = getCycle ( fptr, counter );

				// delete all cycles of length less than 3
				if ( cycle->getLength ( ) >= 3 )
				{
					cycleList->add ( cycle );

					if ( cycle->getLength ( ) > maxCycle ) 
					{
						maxCycle = cycle->getLength ( );
						longCycle = i;
					}
					sumCycle += cycle->getLength ( );
					numCycle++;
				}

				temp = getChar ( fptr, counter );
			}
		}

		rvalue [ i ] = cycleList;
	}
	printf ( "average = %f max = %d\n%d is the longest\n", (float)sumCycle / (float)numCycle, maxCycle, longCycle );

	return rvalue;
}

/** END PARSING ROUTINES **/
Cube::Cube ( char *filename )
{
	FILE *fptr;
	int counter = 0;

	fptr = fopen ( filename, "rt" );
	if ( fptr == NULL ) 
	{
		fprintf ( stderr, "ERROR: could not open %s\n", filename );
		fflush ( stderr );

		exit(0);
	}

	this->cycleTable = getTable ( fptr, counter );
}

Cube::~Cube( void )
{
	for ( int i = 0; i < 256; i++ )
	{
		delete cycleTable [ i ];
	}
	delete cycleTable;
};

LinkedList<Cycle *> * Cube::getCycle ( unsigned char mask )
{
	return this->cycleTable[mask] ;
}