#ifndef LISTITEMSET_H
#define LISTITEMSET_H

#include "Itemset.h"

//ListElement is the element in ListItemset 
class ListElement {
public:
   TM_CALLABLE
   ListElement(Itemset *itemPtr);	// initialize a list element

   TM_CALLABLE
   ~ListElement();

   TM_CALLABLE
   void * operator new(size_t size);

   TM_CALLABLE
   void operator delete(void *);

   TM_CALLABLE
   void set_next(ListElement *n);

   TM_CALLABLE
   Itemset *item();
  
   TM_CALLABLE
   ListElement *next();

   inline void set_item(Itemset *it)
   {
      Item = it;
   }
   
private:
   ListElement *Next;		// next element on list, 
				// NULL if this is the last
   Itemset *Item; 	    	// pointer to item on the list
};

class ListItemset {
public:
   TM_CALLABLE
   ListItemset();			// initialize the list

   TM_CALLABLE
   ~ListItemset();			// de-allocate the list

   TM_CALLABLE
   void * operator new(size_t size);

   TM_CALLABLE
   void operator delete(void *);

   TM_CALLABLE
   void append(Itemset &item); 	// Put item at the end of the list

   TM_CALLABLE
   Itemset *remove(); 	 	// Take item off the front of the list

   ListElement *node(int);
   void sortedInsert(Itemset *);// Put item into list
   ListElement * sortedInsert(Itemset *, ListElement *);
   void clearlist();

   TM_CALLABLE
   int numitems();

   TM_CALLABLE
   ListElement *first();

   friend ostream& operator << (ostream&, ListItemset&);

   inline ListElement *last()
   {
      return Last;
   }

private:
   ListElement *First;  	// Head of the list, NULL if list is empty
   ListElement *Last;		// Last element of list
   int numitem;
};

#endif // LISTITEMSET_H





