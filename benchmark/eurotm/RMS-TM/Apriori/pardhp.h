#ifndef __PARDHP_H
#define __PARDHP_H
#define TM

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdint.h>
#include "tsx.h"

typedef struct TRANSACTION {
  int tid;
  int numitem;
  int *item_list;
} transaction;

typedef struct ATOMIC_SECTION 
{
  int entrance;
  int commit;
  uint64_t max_housekeeping_time;
  uint64_t min_housekeeping_time;
  uint64_t max_executing_time;
  uint64_t min_executing_time;
  uint64_t total_atomicsec_time;
} atomic_section;

typedef struct ALL_ATOMIC_TIME
{
  uint64_t total_housekeeping_time;
  uint64_t total_executing_time;
  uint64_t total_atomic_time;
} all_atomic_time;

using namespace std;
extern transaction *data_set;
extern atomic_section * first_atomic_section;
extern atomic_section * second_atomic_section;
extern atomic_section * third_atomic_section;
extern all_atomic_time * atomic_secs_time;

   
#define CACHE_LNSIZ 128 //128 bytes
   extern int nthreads;
   extern int *hash_indx;
   extern struct timeval tp;

#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))
      
      class prof{
       public:
         double gen;
         double subset;
         double reduce;
         double large;
      };
#ifdef __cplusplus   
   }
#endif

extern TM_PURE uint64_t rdtsc();

#endif //__PARDHP_H
