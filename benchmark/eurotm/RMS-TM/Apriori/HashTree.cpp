#include <stddef.h>
#include "HashTree.h"
#include "pardhp.h"
#include <math.h>
#include <malloc.h>
#include <omp.h>

#define ACQUIRE 0
#define RELEASE 1


TM_PURE
void HashTree::increase_number(int* as_entrance_or_commit_number)
{
  (*as_entrance_or_commit_number)++;
}


TM_CALLABLE
void HashTree::increase_nested_depth(int pid, int * nestedLock_depth,int * max_nested_depth)
{
	nestedLock_depth[pid]++;
        max_nested_depth[pid] = max(nestedLock_depth[pid], max_nested_depth[pid]);
}

TM_CALLABLE
void HashTree::decrease_nested_depth(int pid, int * nestedLock_depth)
{
	nestedLock_depth[pid]--;
}


TM_CALLABLE
void HashTree::atomic_section_times(int pid,atomic_section* atomicsection,ent_ex_times  enter_exit_times_inside,ent_ex_times  enter_exit_times_outside)
{
    uint64_t elapsed;

    uint64_t executing_time = enter_exit_times_inside.exit_atomicsec_time - enter_exit_times_inside.enter_atomicsec_time;

    uint64_t total_time = enter_exit_times_outside.exit_atomicsec_time - 
    enter_exit_times_outside.enter_atomicsec_time;

    uint64_t housekeeping_time = total_time - executing_time;
   
    atomicsection[pid].max_housekeeping_time = max(atomicsection[pid].max_housekeeping_time, housekeeping_time);
    atomicsection[pid].min_housekeeping_time = min(atomicsection[pid].min_housekeeping_time, housekeeping_time);
    atomicsection[pid].max_executing_time = max(atomicsection[pid].max_executing_time, executing_time);
    atomicsection[pid].min_executing_time = min(atomicsection[pid].min_executing_time, executing_time);	

    atomicsection[pid].total_atomicsec_time += total_time;
 
}

TM_CALLABLE
void HashTree::atomic_totalhousekeeping_time(int pid, all_atomic_time * atomic_secs_time, uint64_t inside_enter, uint64_t inside_exit, uint64_t outside_enter, uint64_t outside_exit)
{
	uint64_t executing_time = inside_exit - inside_enter;
	uint64_t total_time = outside_exit - outside_enter;
        atomic_secs_time[pid].total_housekeeping_time += total_time - executing_time;
}

TM_CALLABLE
void HashTree::atomic_totalexecuting_time(int pid, all_atomic_time * atomic_secs_time, uint64_t inside_enter, uint64_t inside_exit)
{
        atomic_secs_time[pid].total_executing_time += inside_exit - inside_enter;
 
}

TM_CALLABLE
void HashTree::atomic_totaltime(int pid, all_atomic_time * atomic_sections_time, uint64_t outside_enter, uint64_t outside_exit)
{
    atomic_sections_time[pid].total_atomic_time += outside_exit - outside_enter;
}

TM_CALLABLE
void HashTree::rearrange_atomic_total_time(int pid, all_atomic_time * atomic_sections_time, uint64_t inside_enter, uint64_t inside_exit, uint64_t outside_enter, uint64_t outside_exit)
{
  atomic_totalhousekeeping_time(pid,atomic_sections_time, inside_enter, inside_exit, outside_enter,outside_exit);
  atomic_totalexecuting_time(pid, atomic_sections_time, inside_enter, inside_exit);
  atomic_totaltime(pid, atomic_sections_time, outside_enter, outside_exit);
}


TM_CALLABLE
void *HashTree::operator new(size_t size) 
{
   HashTree * hashtree = (HashTree*)malloc(size);
   return (void *)hashtree;
}

TM_CALLABLE
void HashTree::operator delete(void *ptr_hashtree) 
{
    HashTree * ptr = (HashTree*) ptr_hashtree;
    free(ptr);
}

TM_CALLABLE
void *HashTree::operator new[] (size_t size) 
{
   HashTree * hashtree = (HashTree*)malloc(size);
   return (void *)hashtree;
}

TM_CALLABLE
void HashTree::operator delete[] (void *ptr_hashtree) 
{
    HashTree * ptr = (HashTree*) ptr_hashtree;
    free(ptr);
}

TM_CALLABLE
HashTree::HashTree (int Depth_P, int hash, int thresh)
{
   Leaf = YES; 
   Count = 0;
   Hash_function = hash;
   Depth = Depth_P;
   List_of_itemsets = NULL;
   Hash_table = (HashTree**) calloc(Hash_function, sizeof(HashTree_Ptr));
   Threshold = thresh;
}

TM_CALLABLE
HashTree::~HashTree()
{
   clear();
}

TM_CALLABLE
void HashTree::clear(){
   if (Leaf){
      if (List_of_itemsets)
         delete List_of_itemsets;
      List_of_itemsets = NULL;
   }
   else{
      if (Hash_table){
         for(int i=0; i < Hash_function; i++){
            if (Hash_table[i]) 
	      delete Hash_table[i];
         }
         free(Hash_table);
         Hash_table = NULL;
      }  
   }
   Leaf = YES; 
   Count = 0;
   Hash_function = 0;
   Threshold = 0;
}

ostream& operator << (ostream& outputStream, HashTree& hashtree){
      if (hashtree.Depth == 0)
         outputStream << " ROOT : C:" << hashtree.Count
                      << " H:" << hashtree.Hash_function << "\n";
      if (hashtree.Leaf){
         if (hashtree.List_of_itemsets != NULL){
            outputStream << " T:" << hashtree.Threshold
                         << " D:" << hashtree.Depth << "\n";
            outputStream << *(hashtree.List_of_itemsets) << flush;
         }
      }
      else{
         for(int i=0; i < hashtree.Hash_function; i++){
            if (hashtree.Hash_table[i]){
#if defined CCPD
               outputStream << "child = " << i << "\n";
#else
               outputStream << "child = " << i
                            << ", Count = " << hashtree.Count << "\n";
#endif
               outputStream << *hashtree.Hash_table[i] << flush;
            }
         }
      }
   
   return outputStream;
}


TM_CALLABLE
int HashTree::hash(int Value)
{
   if(Value != 0)
      return (Value%Hash_function);
   else
      return 0;
}


TM_CALLABLE
void HashTree::rehash(atomic_section * first_atomic_block,atomic_section * second_atomic_block, atomic_section * third_atomic_block,all_atomic_time * atomic_sections_time,int * nestedLock_depth, int *max_nested_depth, int pid)
{
   Leaf = NO;
   Hash_table = (HashTree**) calloc(Hash_function, sizeof(HashTree_Ptr));
   for (int i=0; i < Hash_function; i++)
      Hash_table[i] = NULL;
   
   while(!(List_of_itemsets->first() == NULL)) 
   { // iterate over current itemsets
      Itemset *temp = List_of_itemsets->remove();
#ifdef BALT
      int val = hash_indx[temp->item(Depth)];//according to current Depth
#else
      int val = hash(temp->item(Depth)); // according to current Depth
#endif

      if (Hash_table[val]==NULL){
         Hash_table[val] = new HashTree(Depth+1, Hash_function, Threshold);
      }
      Hash_table[val]->add_element(*temp,first_atomic_block, second_atomic_block, third_atomic_block,atomic_sections_time,nestedLock_depth,max_nested_depth, pid);
   }

   delete List_of_itemsets;
   List_of_itemsets = NULL;
}



TM_CALLABLE
int HashTree::add_element(Itemset& Value, atomic_section * first_atomic_block,atomic_section * second_atomic_block,atomic_section * third_atomic_block, all_atomic_time * atomic_sections_time,int * nestedLock_depth, int *max_nested_depth, int pid)
{
//Gokcen
	ent_ex_times enter_exit_times_outside;
	ent_ex_times enter_exit_times_inside;
//Gokcen_end
  
   
   if (Leaf)
   {
#if defined CCPD
      enter_exit_times_outside.enter_atomicsec_time = rdtsc();
      TRANSACTION_BEGIN
		enter_exit_times_inside.enter_atomicsec_time = rdtsc();
		increase_number(&(first_atomic_block[pid].entrance));
		increase_nested_depth(pid,nestedLock_depth,max_nested_depth);
		if (List_of_itemsets == NULL)
			List_of_itemsets = new ListItemset();
		
		List_of_itemsets->append(Value);
		if(List_of_itemsets->numitems() > Threshold){
			if (Depth+1 > Value.numitems())
			Threshold++;
			else 
			{
			rehash(first_atomic_block, second_atomic_block, third_atomic_block,atomic_sections_time, nestedLock_depth,max_nested_depth,pid); 
			}  // if so rehash
		}
		enter_exit_times_inside.exit_atomicsec_time = rdtsc();
      TRANSACTION_END
      enter_exit_times_outside.exit_atomicsec_time = rdtsc();
  
      /*Gokcen*/
      decrease_nested_depth(pid, nestedLock_depth);
      if (nestedLock_depth[pid] == 0)
      {
        atomic_section_times(pid,first_atomic_block,enter_exit_times_inside,enter_exit_times_outside);
        
        rearrange_atomic_total_time(pid, atomic_sections_time, enter_exit_times_outside.enter_atomicsec_time, enter_exit_times_outside.exit_atomicsec_time, 
        enter_exit_times_inside.enter_atomicsec_time, 
        enter_exit_times_inside.exit_atomicsec_time);

      }
#else
    if (List_of_itemsets == NULL)
         List_of_itemsets = new ListItemset;
      
     
      List_of_itemsets->append(Value);
      if(List_of_itemsets->numitems() > Threshold){
         if (Depth+1 > Value.numitems())
            Threshold++;
         else 
          {
            rehash(first_atomic_block, second_atomic_block, third_atomic_block,atomic_sections_time, nestedLock_depth,max_nested_depth,pid); 
          }  // if so rehash
      }
#endif

   }
   else
   {
#ifdef BALT
      int val = hash_indx[Value.item(Depth)];
#else
      int val = hash(Value.theItemset[Depth]);
#endif

	if (Hash_table[val] == NULL)
	{
	   	#if defined CCPD
                enter_exit_times_outside.enter_atomicsec_time = rdtsc();
                TRANSACTION_BEGIN
			enter_exit_times_inside.enter_atomicsec_time = rdtsc();
			increase_number(&(second_atomic_block[pid].entrance));
	  	 #endif
			if (Hash_table[val] == NULL)
			Hash_table[val] = new HashTree(Depth+1, Hash_function, Threshold);
	   	#if defined CCPD
			enter_exit_times_inside.exit_atomicsec_time = rdtsc();

               TRANSACTION_END
			enter_exit_times_outside.exit_atomicsec_time = rdtsc();
		
	                atomic_section_times(pid,second_atomic_block,enter_exit_times_inside,enter_exit_times_outside);
		
			rearrange_atomic_total_time(pid, atomic_sections_time, enter_exit_times_outside.enter_atomicsec_time, enter_exit_times_outside.exit_atomicsec_time, 
			enter_exit_times_inside.enter_atomicsec_time, 
			enter_exit_times_inside.exit_atomicsec_time);

		#endif
	}
      Hash_table[val]->add_element(Value,first_atomic_block, second_atomic_block,third_atomic_block,atomic_sections_time,nestedLock_depth,max_nested_depth, pid);
   }
#if defined CCPD
   if (is_root())
   {
      int tt;

      enter_exit_times_outside.enter_atomicsec_time = rdtsc();
      TRANSACTION_BEGIN
	enter_exit_times_inside.enter_atomicsec_time = rdtsc();
	increase_number(&(third_atomic_block[pid].entrance));
			Count++;
			tt = Count;
	
	enter_exit_times_inside.exit_atomicsec_time = rdtsc();
      TRANSACTION_END
      enter_exit_times_outside.exit_atomicsec_time = rdtsc();

      atomic_section_times(pid,third_atomic_block,enter_exit_times_inside,enter_exit_times_outside);
 
      rearrange_atomic_total_time(pid, atomic_sections_time, 
      enter_exit_times_outside.enter_atomicsec_time, 
      enter_exit_times_outside.exit_atomicsec_time, 
      enter_exit_times_inside.enter_atomicsec_time, 
      enter_exit_times_inside.exit_atomicsec_time);

      return tt;
   }
#else
   Count++;
   return Count;
#endif
}	

