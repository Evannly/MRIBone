#ifndef CYCLE_H
#define CYCLE_H

#include "LinkedList.h"

class Cycle {
private:
	/// the length of the cycle
	int length;
	/// the array for the cycle
	char *cycleArray;

public:
	/**
	 * Constructor.  Generates a cycle from the given cycleList
	 *
	 * @param cycleList a pointer to a linked list that this cycle should
	 *                  be filled with
	 */
	Cycle ( LinkedList<char> *cycleList )
	{
		length = cycleList->getLength ( );
		cycleArray = cycleList->toArray ( );
	}

	/**
	 * Constructor.  Makes an empty cycle of length size.
	 *
	 * @param size the size to make this empty cycle
	 */
	Cycle ( int size ) 
	{
		length = size;
		cycleArray = new char [ size ];
	}

	/**
	 * Destructor
	 */
	~Cycle ( void )
	{
		delete cycleArray;
	}

	/**
	 * Returns the length of this cycle
	 *
	 * @return the length of this cycle
	 */
	int getLength ( void )
	{
		return length;
	}

	/**
	 * Returns the edge at the given index
	 *
	 * @param index the index to lookup the required edge
	 *
	 * @return the edge at the associated index
	 */
	int getEdge ( int index )
	{
		return (int) (cycleArray [ index ]);
	}

	/**
	 * Sets the edge at the given index
	 *
	 * @param index the index to set the edge at
	 * @param pair what to put at the given index
	 */
	void setEdge ( int index, int edge ) 
	{
		cycleArray [ index ] = (char) edge;
	}

};
#endif