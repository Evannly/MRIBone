#ifndef LINKED_LIST_H
#define LINKED_LIST_H

#include <stdio.h>

/**
 * LinkedList class... actually, it's a doubly linked list
 *
 * @author Scott Schaefer
 */

/**
 * Node for the linked list class.  No one should use this 
 * except the linked list class
 */
template <class T>
class LinkedListNode
{
	protected:
		/// the data to store
		T data;

	public:
		/// the next element in the list
		LinkedListNode *next;
		/// the previous element in the list
		LinkedListNode *prev;

	public:
		/**
		 * Constructor
		 *
		 * @param value the value to store in this node
		 */
		LinkedListNode ( const T& value )
		{
			data = value;
			next = this;
			prev = this;
		}

		/**
		 * Returns the next node in the list
		 * 
		 * @return the next node in the list
		 */
		LinkedListNode *getNext ( void ) 
		{
			return next;
		}

		/**
		 * Returns the data in this node
		 * 
		 * @return the data stored in this node
		 */
		T getData ( void )
		{
			return data;
		}
};

/**
 * LinkedList class
 */
template <class T>
class LinkedList
{
	protected:
		/// the length of the list
		int length;
		/// the list and a mark pointer
		LinkedListNode<T> *list, *mark;

	public:
		/**
		 * Constructor
		 */
		LinkedList ( void )
		{
			list = NULL;
			mark = NULL;
			length = 0;
		}

		/**
		 * Constructor that creates a linked list containing one value
		 *
		 * @param value the value to store in the linked list
		 */
		LinkedList ( const T& value )
		{
			list = new LinkedListNode<T> ( value );
			mark = NULL;
			length = 1;
		}

		/**
		 * Destructor: frees up the memory used
		 */
		~LinkedList ( void )
		{
			clearList ( );
		}

		/**
		 * Frees all memory used by the list and 
		 * sets the list back to an empty list
		 */
		void clearList ( void )
		{
			LinkedListNode<T> *ptr, *ptr1;
			int i;

			ptr = list;
			for ( i = 0; i < length; i++ )
			{
				ptr1 = ptr;
				ptr = ptr->next;

				delete ptr1;
			}

			list = NULL;
			mark = NULL;
			length = 0;
		}

		/**
		 * Marks the current position in the list so that it can be
		 * jumped to later using jumpToMark
		 */
		void markPosition ( void )
		{
			mark = list;
		}

		/**
		 * Jumps to a previously marked position.  Note that certain
		 * operations invalidate the marked position such as deleting
		 * elements.
		 */
		void jumpToMark ( void )
		{
			if ( mark != NULL )
			{
				list = mark;
			}
		}

		/**
		 * Returns the length of the list.  This is an O(1) algorithm,
		 * not O(n)
		 *
		 * @return the length of the list
		 */
		int getLength ( void )
		{
			return length;
		}

		/**
		 * Returns the first item in this list.
		 *
		 * @return the first item in this list
		 */
		T getFirst ( void )
		{
			return list->getData ( );
		}

		/**
		 * Rotates the list to the right
		 */
		void rotateRight ( void )
		{
			if ( length == 0 )
			{
				return;
			}

			list = list->prev;
		}

		/**
		 * Rotates the list to the left
		 */
		void rotateLeft ( void )
		{
			if ( length == 0 ) 
			{
				return;
			}

			list = list->next;
		}

		/**
		 * Adds an element to the list
		 *
		 * @param value the element to add to the list
		 */
		void add ( const T& value )
		{
			if ( length == 0 )
			{
				list = new LinkedListNode<T> ( value );
			}
			else
			{
				LinkedListNode<T> *prev = list->prev;
				LinkedListNode<T> *node = new LinkedListNode<T> ( value );

				list->prev = node;
				prev->next = node;

				node->next = list;
				node->prev = prev;
			}

			length++;
		}

		/**
		 * Removes the given element from the list if it is in the list
		 *
		 * @param value the element to try to remove from the list
		 */
		void remove ( const T& value )
		{
			LinkedListNode<T> *ptr;

			if ( length == 0 )
			{
				return;
			}

			ptr = list;

			do 
			{
				if ( ptr->getData ( ) == value )
				{
					if ( ptr == list )
					{
						// deleting the head item
						deleteFront ( );
						return;
					}
					else
					{
						// deleting some random item in the list
						ptr->next->prev = ptr->prev;
						ptr->prev->next = ptr->next;

						length--;

						mark = NULL;
						
						delete ptr;
						return;
					}
				}

				ptr = ptr->getNext ( );
			} while ( ptr != list );
		}

		/**
		 * Returns and deletes the first element from the list
		 */
		int pop ( T& rvalue )
		{
			if ( length == 0 )
			{
				return 0;
			}

			mark = NULL;
			if ( length == 1 )
			{
				// only one element in the list!
				rvalue = list->getData() ;
				delete list;
				list = NULL;
			}
			else
			{
				LinkedListNode<T> *old = list;
				list->prev->next = list->next;
				list->next->prev = list->prev;
				list = list->next;

				rvalue = old->getData() ;
				delete old;
			}
			
			length--;

			return 1 ;
		}

		/**
		 * Deletes the first element from the list
		 */
		void deleteFront ( void )
		{
			if ( length == 0 )
			{
				return;
			}

			mark = NULL;

			if ( length == 1 )
			{
				// only one element in the list!
				delete list;

				list = NULL;
			}
			else
			{

				LinkedListNode<T> *old = list;

				list->prev->next = list->next;
				list->next->prev = list->prev;

				list = list->next;

				delete old;
			}
			
			length--;
		}

		/**
		 * Appends the given list onto this list.  The given list is 
		 * emptied in this process.
		 *
		 * @param linkedlist the list to append onto this list
		 */
		void append ( LinkedList<T> *linkedlist )
		{
			if ( linkedlist == NULL || linkedlist->list == NULL ) 
			{
				return;
			}

			if ( list == NULL ) 
			{
				list = linkedlist->list;

				length = linkedlist->getLength ( );

				// kill the old list
				linkedlist->list = NULL;
				linkedlist->length = 0;
				return;
			}

			LinkedListNode<T> *oldEnd, *oldBegin, *newEnd, *newBegin;

			newBegin = list;
			newEnd = list->prev;

			oldEnd = linkedlist->list->prev;
			oldBegin = linkedlist->list;

			newBegin->prev = oldEnd;
			oldEnd->next = newBegin;

			newEnd->next = oldBegin;
			oldBegin->prev = newEnd;

			length += linkedlist->getLength ( );

			// kill the old list
			linkedlist->list = NULL;
			linkedlist->length = 0;
		}

		/**
		 * Converts this list into an array.  Note that the returned array
		 * needs to be deleted so that a memory leak isn't caused.
		 *
		 * @return an array filled with the data contained in this list
		 */
		T* toArray ( void )
		{
			T* rvalue = new T [ length ];
			int i;
			LinkedListNode<T> *ptr = list;

			for ( i = 0; i < length; i++ ) 
			{
				rvalue [ i ] = ptr->getData ( );
				ptr = ptr->getNext ( );
			}

			return rvalue;
		}
};

#endif