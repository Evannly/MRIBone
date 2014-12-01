#ifndef HASHMAP_H
#define HASHMAP_H

#include <iostream>
#include <stdio.h>

/* A hash table for grid edges (3 int)
 *
 */

const int H_LENGTH = 18;
const int MAX_H = 1<<18;
const int H_BIT_1 = 6;
const int H_BIT_2 = 6;
const int H_BIT_3 = 6;

struct HashElement
{
/// Key of hash element
	int key[3];
/// Actually content of hash element
	int index ;
	float x,y,z;
/// Link when collision
	HashElement  *nextHash;
};

class HashMap
{
	/// Hash table
	HashElement *table[MAX_H];

	/// Create hash key
	int createKey( int k1, int k2, int k3 )
	{
		int ind = 
			((( k1 & ( (1<<H_BIT_1) - 1 )) << ( H_BIT_3 + H_BIT_2 )) | 
			(( k2 & ( (1<<H_BIT_2) - 1 )) << ( H_BIT_3  )) |
			( k3 & ( (1<<H_BIT_3) - 1)) ) & ((1<<H_LENGTH) - 1);

		return ind;
	}

public:

	/// Constructor
	HashMap ( )
	{
		for (int i = 0; i < MAX_H; i ++)
		{
			table[i] = NULL;
		}
	};

	/// Lookup Method
	int find( int k1, int k2, int k3, int& index, float& x, float& y, float& z )
	{
		/// Create hash key
		int ind = createKey ( k1, k2, k3 );

		/// Find it in the table
		HashElement *p = table[ind];
		while (p)
		{
			if ((p->key[0] == k1) && (p->key[1] == k2) && (p->key[2] == k3))
			{
				index = p->index ;
				x = p->x ;
				y = p->y ;
				z = p->z ;
				return 1 ;
			}
			p = p->nextHash;
		}

		// Not found
		return 0 ;
	};
	
	/// Lookup Method
	void insert( int k1, int k2, int k3, int index, float x, float y, float z )
	{
		/// Create hash key
		int ind = createKey ( k1, k2, k3 );

		/// Put it in the table
		HashElement *p = new HashElement ;
		p->key[0] = k1;
		p->key[1] = k2;
		p->key[2] = k3;
		p->index = index ;
		p->x = x ;
		p->y = y ;
		p->z = z ;
		p->nextHash = table[ind];
		table[ind] = p;
	};

	// Destruction method
	~HashMap()
	{
		HashElement *p, *pp;

		for (int i = 0; i < MAX_H; i ++)
		{
			p = table[i];

			while (p)
			{
				pp = p->nextHash;
				delete p;
				p = pp;
			}

		}

	};

};


#endif