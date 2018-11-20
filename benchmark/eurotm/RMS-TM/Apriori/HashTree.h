#ifndef __HASHTREE_H
#define __HASHTREE_H
#include <omp.h>
#include "ListItemset.h"
#define CCPD
#define YES 1 
#define NO 0 


typedef struct ENTER_EXIT_TIMES
{
  uint64_t enter_atomicsec_time;
  uint64_t exit_atomicsec_time;
} ent_ex_times;

class HashTree 
{
public:
    
   TM_CALLABLE   
   HashTree(int, int, int);
    
   TM_CALLABLE
   ~HashTree();

   TM_CALLABLE
   void clear();

   TM_CALLABLE
   void rehash(atomic_section *,atomic_section *,atomic_section *,all_atomic_time *,int*, int*,int); // procedure for converting leaf node to a interior node
   
   TM_CALLABLE
   int hash(int); // procedure to find out which node item hashes to
    
   TM_CALLABLE  
   void *operator new(size_t);

   TM_CALLABLE
   void operator delete(void *);

   TM_CALLABLE  
   void *operator new[](size_t);

   TM_CALLABLE
   void operator delete[](void *);
   
   
   TM_CALLABLE
   int add_element(Itemset&,atomic_section *,atomic_section *,atomic_section *,all_atomic_time *, int*, int*, int);

   
   TM_CALLABLE
   int is_root(){return (Depth == 0);};

   //Gokcen
 
   TM_PURE
   void increase_number(int*);

   TM_CALLABLE
   void increase_nested_depth(int, int *,int *);

   TM_CALLABLE
   void decrease_nested_depth(int, int*);
   
   TM_CALLABLE
   void atomic_section_times(int,atomic_section*,ent_ex_times,ent_ex_times);

   TM_CALLABLE
   void atomic_totalhousekeeping_time(int, all_atomic_time *, uint64_t, uint64_t, uint64_t, uint64_t); 
   
   TM_CALLABLE
   void atomic_totalexecuting_time(int, all_atomic_time *, uint64_t, uint64_t); 
   
   TM_CALLABLE
   void atomic_totaltime(int, all_atomic_time *, uint64_t, uint64_t); 

   TM_CALLABLE
   void rearrange_atomic_total_time(int, all_atomic_time *, uint64_t, uint64_t, uint64_t, uint64_t);
   

   //Gokcen_end
   friend ostream& operator << (ostream&, HashTree&);

   inline int is_leaf()
   {
      return (Leaf == YES);
   }

   inline ListItemset * list()
   {
      return List_of_itemsets;
   }

   inline int hash_function()
   {
      return Hash_function;
   }

   inline int depth()
   {
      return Depth;
   }

   inline int hash_table_exists()
   {
      return (Hash_table != NULL);
   }
   
   inline HashTree *hash_table(int pos)
   {
      return Hash_table[pos];
   }
   
private:
   int Leaf;
   HashTree **Hash_table;
   int Hash_function;
   int Depth;
   ListItemset *List_of_itemsets;
   int Count;
   int Threshold; 
};

typedef HashTree * HashTree_Ptr;
#endif //__HASHTREE_H
