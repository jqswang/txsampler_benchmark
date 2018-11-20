/*
 * File:
 *   intset.c
 * Author(s):
 *   Vincent Gramoli <vincent.gramoli@epfl.ch>
 * Description:
 *   Integer set operations accessing the hashtable
 *
 * Copyright (c) 2009-2010.
 *
 * intset.c is part of Synchrobench
 *
 * Synchrobench is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, version 2
 * of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include "intset.h"


#define LTX_NODE_TYPE node_t
#define ITERATION_NUMBER_GENERATOR (rand()%30 + 10)
#include "large_tx.h"

int ht_contains(ht_intset_t *set, int val, int transactional) {
	int addr;

	addr = val % maxhtlength;
	if (transactional == 5)
	  return set_contains(set->buckets[addr], val, 4);
	else
	  return set_contains(set->buckets[addr], val, transactional);
}

int ht_add(ht_intset_t *set, int val, int transactional) {
	int addr;

	addr = val % maxhtlength;
	if (transactional == 5)
		return set_add(set->buckets[addr], val, 4);
	else
		return set_add(set->buckets[addr], val, transactional);
}

int ht_remove(ht_intset_t *set, int val, int transactional) {
	int addr;

	addr = val % maxhtlength;
	if (transactional == 5)
		return set_remove(set->buckets[addr], val, 4);
	else
		return set_remove(set->buckets[addr], val, transactional);
}

/*
 * Move an element from one bucket to another.
 * It is equivalent to changing the key associated with some value.
 *
 * This version allows the removal of val1 while insertion of val2 fails (e.g.
 * because val2 already present. (Despite this partial failure, the move returns
 * true.) As a result, the data structure size may decrease as moves execute.
 */
int ht_move_naive(ht_intset_t *set, int val1, int val2, int transactional) {
	int result = 0;

#ifdef SEQUENTIAL

	int addr1, addr2;

	addr1 = val1 % maxhtlength;
	addr2 = val2 % maxhtlength;
	result =  (set_remove(set->buckets[addr1], val1, transactional) &&
			   set_add(set->buckets[addr2], val2, transactional));

#elif defined STM

	int v, addr1, addr2;
	node_t *n, *prev, *next;

	if (transactional > 1) {

	  TX_START(EL);
	  addr1 = val1 % maxhtlength;
	  prev = (node_t *)TX_LOAD(&set->buckets[addr1]->head);
	  next = (node_t *)TX_LOAD(&prev->next);
	  while(1) {
	    v = TX_LOAD(&next->val);
	    if (v >= val1) break;
	    prev = next;
	    next = (node_t *)TX_LOAD(&prev->next);
	  }
	  if (v == val1) {
	    /* Physically removing */
	    n = (node_t *)TX_LOAD(&next->next);
	    TX_STORE(&prev->next, n);
	    FREE(next, sizeof(node_t));
	    /* Inserting */
	    addr2 = val2 % maxhtlength;
	    prev = (node_t *)TX_LOAD(&set->buckets[addr2]->head);
	    next = (node_t *)TX_LOAD(&prev->next);
	    while(1) {
	      v = TX_LOAD(&next->val);
	      if (v >= val2) break;
	      prev = next;
	      next = (node_t *)TX_LOAD(&prev->next);
	    }
	    if (v != val2) {
	      TX_STORE(&prev->next, new_node(val2, next, transactional));
	    }
	    /* Even if the key is already in, the operation succeeds */
	    result = 1;
	  } else result = 0;
	  TX_END;

	} else {

	  TX_START(NL);
	  addr1 = val1 % maxhtlength;
	  prev = (node_t *)TX_LOAD(&set->buckets[addr1]->head);
	  next = (node_t *)TX_LOAD(&prev->next);
	  while(1) {
	    v = TX_LOAD(&next->val);
	    if (v >= val1) break;
	    prev = next;
	    next = (node_t *)TX_LOAD(&prev->next);
	  }
	  if (v == val1) {
	    /* Physically removing */
	    n = (node_t *)TX_LOAD(&next->next);
	    TX_STORE(&prev->next, n);
	    FREE(next, sizeof(node_t));
	    /* Inserting */
	    addr2 = val2 % maxhtlength;
	    prev = (node_t *)TX_LOAD(&set->buckets[addr2]->head);
	    next = (node_t *)TX_LOAD(&prev->next);
	    while(1) {
	      v = TX_LOAD(&next->val);
	      if (v >= val2) break;
	      prev = next;
	      next = (node_t *)TX_LOAD(&prev->next);
	    }
	    if (v != val2) {
	      TX_STORE(&prev->next, new_node(val2, next, transactional));
	    }
	    /* Even if the key is already in, the operation succeeds */
	    result = 1;
	  } else result = 0;
	  TX_END;

	}

#elif defined LOCKFREE /* No CAS-based implementation is provided */

	printf("ht_snapshot: No other implementation of atomic snapshot is available\n");
	exit(1);

#endif

	return result;
}


/*
 * This version parses the data structure twice to find appropriate values
 * before updating it.
 */
int ht_move(ht_intset_t *set, int val1, int val2, int transactional) {
  int result = 0;

#ifdef SEQUENTIAL

	int addr1, addr2;

	addr1 = val1 % maxhtlength;
	addr2 = val2 % maxhtlength;

	if (set_remove(set->buckets[addr1], val1, 0))
	  result = 1;
	set_seq_add(set->buckets[addr2], val2, 0);
	return result;

#elif defined STM

	int v, addr1, addr2;
	node_t *n=NULL, *prev=NULL, *next=NULL, *prev1=NULL,  *next1=NULL, *prev2 = NULL, *next2 = NULL;
	//printf("move %d to %d\n", val1, val2);

  result = -1;
  addr1 = val1 % maxhtlength;
	addr2 = val2 % maxhtlength;
	int choice;
	if (addr1 < addr2) choice = 1; //find the old value first
	else if (addr1 > addr2) choice = 2;//find the new value first
	else { //addr1 == addr2
		if (val1 <= val2) choice = 1;
		else choice = 2;
	}
	int i;
	for( i=0; i < 2; i++) {
		if (choice == 1){ //locate the old value
			LARGE_TX_BEGIN(set->buckets[addr1]->head);
			prev = (node_t *)TX_LOAD(&set->buckets[addr1]->head);
			//next = (node_t *)TX_LOAD(&prev->next);
			HT_MOVE_OLD_TX:
			while(1) {
				next = (node_t *)TX_LOAD(&prev->next);
				v = TX_LOAD(&next->val);
				if (v >= val1) break;
				LARGE_TX_BREAK;
				prev = next;
				//next = (node_t *)TX_LOAD(&prev->next);
			}
			if (v != val1) {
				LARGE_TX_END;
				result = 0;
				break;
			}
			prev1 = prev;
			next1 = next;
			if (prev1 == prev2){
				LARGE_TX_END;
			}
			else if (prev1 == next2){
				LARGE_TX_ACQUIRE_ONE_NODE_LOCK_AND_END(next1,HT_MOVE_OLD_TX);
			}
			else if (next1 == prev2){
				LARGE_TX_ACQUIRE_ONE_NODE_LOCK_AND_END(prev1,HT_MOVE_OLD_TX);
			}
			else {
				LARGE_TX_ACQUIRE_TWO_NODES_LOCK_AND_END(prev1,next1,HT_MOVE_OLD_TX);
			}
			choice = 2;
			continue;
		}
		if (choice == 2) { //locate the new value
			LARGE_TX_BEGIN(set->buckets[addr2]->head);
			prev = (node_t *)TX_LOAD(&set->buckets[addr2]->head);
			HT_MOVE_NEW_TX:
			while(1) {
				next = (node_t *)TX_LOAD(&prev->next);
				v = TX_LOAD(&next->val);
				if (v >= val2) break;
				LARGE_TX_BREAK;
				prev = next;
				//next = (node_t *)TX_LOAD(&prev->next);
			}
			prev2 = prev;
			next2 = next;
			if (prev1 == prev2){
				LARGE_TX_END;
			}
			else if (prev1 == next2){
				LARGE_TX_ACQUIRE_ONE_NODE_LOCK_AND_END(prev2,HT_MOVE_NEW_TX);
			}
			else if (next1 == prev2){
				LARGE_TX_ACQUIRE_ONE_NODE_LOCK_AND_END(next2,HT_MOVE_NEW_TX);
			}
			else {
				LARGE_TX_ACQUIRE_TWO_NODES_LOCK_AND_END(prev2,next2,HT_MOVE_NEW_TX);
			}
			choice = 1;
			continue;
		}
	}
	if (result != 0){
		if (v != val2 && prev2 != prev1 && prev2 != next1) {
			/* Even if the key is already in, the operation succeeds */
			result = 1;
			TM_BEGIN();
			/* Physically removing */
			n = (node_t *)TX_LOAD(&next1->next);
			TX_STORE(&prev1->next, n);
			TX_STORE(&prev2->next, new_node(val2, next2, transactional));
			RELEASE_NODE_LOCK_IF_ACQUIRED(prev1);
			RELEASE_NODE_LOCK_IF_ACQUIRED(next1);
			RELEASE_NODE_LOCK_IF_ACQUIRED(prev2);
			RELEASE_NODE_LOCK_IF_ACQUIRED(next2);
			TM_END();
			FREE(next1, sizeof(node_t));
		}
		else {
			TM_BEGIN();
			RELEASE_NODE_LOCK_IF_ACQUIRED(prev1);
			RELEASE_NODE_LOCK_IF_ACQUIRED(next1);
			RELEASE_NODE_LOCK_IF_ACQUIRED(prev2);
			RELEASE_NODE_LOCK_IF_ACQUIRED(next2);
			TM_END();
			result = 0;
		}
	}
	else {
		TM_BEGIN();
		RELEASE_NODE_LOCK_IF_ACQUIRED(prev1);
		RELEASE_NODE_LOCK_IF_ACQUIRED(next1);
		RELEASE_NODE_LOCK_IF_ACQUIRED(prev2);
		RELEASE_NODE_LOCK_IF_ACQUIRED(next2);
		TM_END();
	}

#elif defined LOCKFREE /* No CAS-based implementation is provided */

	printf("ht_snapshot: No other implementation of atomic snapshot is available\n");
	exit(1);

#endif

	return result;
}

/*
 * This version removes val1 it finds first and re-insert this value if it does
 * not succeed in inserting val2 so that it can insert somewhere (for the size
 * to remain unchanged).
 */
int ht_move_orrollback(ht_intset_t *set, int val1, int val2, int transactional) {
  int result = 0;

#ifdef SEQUENTIAL

	int addr1, addr2;
	addr1 = val1 % maxhtlength;
	addr2 = val2 % maxhtlength;
	result =  (set_remove(set->buckets[addr1], val1, transactional) &&
			   set_add(set->buckets[addr2], val2, transactional));

#elif defined STM

	int v, addr1, addr2;
	node_t *n, *prev, *next, *prev1, *next1;

	if (transactional > 1) {

	  TX_START(EL);
	  result = 0;
	  addr1 = val1 % maxhtlength;
	  prev = (node_t *)TX_LOAD(&set->buckets[addr1]->head);
	  next = (node_t *)TX_LOAD(&prev->next);
	  while(1) {
	    v = TX_LOAD(&next->val);
	    if (v >= val1) break;
	    prev = next;
	    next = (node_t *)TX_LOAD(&prev->next);
	  }
	  prev1 = prev;
	  next1 = next;
	  if (v == val1) {
	    /* Physically removing */
	    n = (node_t *)TX_LOAD(&next->next);
	    TX_STORE(&prev->next, n);
	    /* Inserting */
	    addr2 = val2 % maxhtlength;
	    prev = (node_t *)TX_LOAD(&set->buckets[addr2]->head);
	    next = (node_t *)TX_LOAD(&prev->next);
	    while(1) {
	      v = TX_LOAD(&next->val);
	      if (v >= val2) break;
	      prev = next;
	      next = (node_t *)TX_LOAD(&prev->next);
	    }
	    if (v != val2) {
	      TX_STORE(&prev->next, new_node(val2, next, transactional));
	      FREE(next1, sizeof(node_t));
	    } else {
	      TX_STORE(&prev1->next, TX_LOAD(&next1));
	    }
	    /* Even if the key is already in, the operation succeeds */
	    result = 1;
	  } else result = 0;
	  TX_END;


	} else {

	  TX_START(NL);
	  result = 0;
	  addr1 = val1 % maxhtlength;
	  prev = (node_t *)TX_LOAD(&set->buckets[addr1]->head);
	  next = (node_t *)TX_LOAD(&prev->next);
	  while(1) {
	    v = TX_LOAD(&next->val);
	    if (v >= val1) break;
	    prev = next;
	    next = (node_t *)TX_LOAD(&prev->next);
	  }
	  prev1 = prev;
	  next1 = next;
	  if (v == val1) {
	    /* Physically removing */
	    n = (node_t *)TX_LOAD(&next->next);
	    TX_STORE(&prev->next, n);
	    /* Inserting */
	    addr2 = val2 % maxhtlength;
	    prev = (node_t *)TX_LOAD(&set->buckets[addr2]->head);
	    next = (node_t *)TX_LOAD(&prev->next);
	    while(1) {
	      v = TX_LOAD(&next->val);
	      if (v >= val2) break;
	      prev = next;
	      next = (node_t *)TX_LOAD(&prev->next);
	    }
	    if (v != val2) {
	      TX_STORE(&prev->next, new_node(val2, next, transactional));
	      FREE(next1, sizeof(node_t));
	    } else {
	      TX_STORE(&prev1->next, TX_LOAD(&next1));
	    }
	    /* Even if the key is already in, the operation succeeds */
	    result = 1;
	  } else result = 0;
	  TX_END;

	}

#elif defined LOCKFREE /* No CAS-based implementation is provided */

	printf("ht_snapshot: No other implementation of atomic snapshot is available\n");
	exit(1);

#endif

	return result;
}


/*
 * Atomic snapshot of the hash table.
 * It parses the whole hash table to sum all elements.
 *
 * Observe that this particular operation (atomic snapshot) cannot be implemented using
 * elastic transactions in combination with the move operation, however, normal transactions
 * compose with elastic transactions.
 */
int ht_snapshot(ht_intset_t *set, int transactional) {
  int result = 0;

#ifdef SEQUENTIAL

	int i, sum = 0;
	node_t *next;

	for (i=0; i < maxhtlength; i++) {
		next = set->buckets[i]->head->next;
		while(next->next) {
			sum += next->val;
			next = next->next;
		}
	}
	result = 1;

#elif defined STM

	int i, sum = 0, sum_backup = 0;
	node_t *prev;

	// always a normal transaction
	result = 0;
	for (i=0; i < maxhtlength; i++) {
		LARGE_TX_BEGIN(set->buckets[i]->head->next);
		sum_backup = sum;
		prev = (node_t *)TX_LOAD(&set->buckets[i]->head->next);
		HT_SNAPSHOT_TX:
		while(prev->next) {
			LARGE_TX_BEFORE_SUM(HT_SNAPSHOT_TX);
			sum += TX_LOAD(&prev->val);
			LARGE_TX_BREAK_SUM;
			prev = (node_t *)TX_LOAD(&prev->next);
		}
		LARGE_TX_END;
	}
	result = 1;

#elif defined LOCKFREE /* No CAS-based implementation is provided */

	printf("ht_snapshot: No other implementation of atomic snapshot is available\n");
	exit(1);

#endif

	return result;
}
