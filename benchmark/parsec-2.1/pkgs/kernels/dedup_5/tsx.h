#ifndef TSX_H
#define TSX_H

#ifdef RTM
#include "tm.h"

#ifdef GLOBAL_LOCK
extern TM_LOCK_T external_global_lock;
#define pthread_mutex_t        /* nothing */
#define pthread_mutex_init(mutex,attr)      /* nothing */
#define pthread_mutex_destroy(mutex)  /* nothing */
#define pthread_mutex_lock(mutex)     TM_BEGIN_ARGS(&external_global_lock)
#define pthread_mutex_unlock(mutex)    TM_END_ARGS(&external_global_lock)
#define pthread_cond_t          /* nothing */
#define pthread_cond_init(cond,attr)      /* nothing */
#define pthread_cond_destroy(cond)    /* nothing */
#define pthread_cond_signal(cond) /*nothing*/
#define pthread_cond_broadcast(cond)  /*nothing*/
#define pthread_cond_wait(cond, mutex)  {TM_END_ARGS(&external_global_lock);TM_BEGIN_ARGS(&external_global_lock);}

#else

#define pthread_mutex_t        /* nothing */
#define pthread_mutex_init(mutex,attr)      /* nothing */
#define pthread_mutex_destroy(mutex)  /* nothing */
#define pthread_mutex_lock(mutex)     TM_BEGIN_ARGS(mutex)
#define pthread_mutex_unlock(mutex)    TM_END_ARGS(mutex)
#define pthread_cond_t          /* nothing */
#define pthread_cond_init(cond,attr)      /* nothing */
#define pthread_cond_destroy(cond)    /* nothing */
#define pthread_cond_signal(cond) TM_COND_SIGNAL_SPIN(cond)
#define pthread_cond_broadcast(cond)  TM_COND_BROADCAST_SPIN(cond)
#define pthread_cond_wait(cond, mutex)  {TM_END_ARGS(mutex);TM_BEGIN_ARGS(mutex);}
#endif // GLOBAL_LOCK

#else
#define TM_BEGIN() /* nothing */
#define TM_END() /* nothing */
#define TM_THREAD_ENTER() /* nothing */
#define TM_THREAD_EXIT() /* nothing */
#define TM_STARTUP(threads_num) /* nothing */
#define TM_SHUTDOWN() /* nothing */
#endif /*RTM */

#endif /*TSX_H*/
