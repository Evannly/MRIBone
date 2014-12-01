#ifndef POLYTOPE_H
#define POLYTOPE_H

#include <stdio.h>
#include "Cycle.h"
#include "GeoCommon.h"

class Cube {
public:
	/// the cycle table
	LinkedList<Cycle*> **cycleTable;
	
public:
/**
* Constructor.
	*/
	Cube ( char *filename );
	
	/**
	* Destructor
	*/
	~Cube ( void );
	
	/**
	* Returns cycles for a particular sign configuration
	*/
	LinkedList<Cycle*>* getCycle ( unsigned char mask ) ;
	
	
};

#endif