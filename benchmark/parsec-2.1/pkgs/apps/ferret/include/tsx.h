#ifndef TSX_H
#define TSX_H

#include <pthread.h>

#ifdef RTM
#include "tm.h"

#ifdef FINE_GRAINED_LOCK
#define pthread_mutex_t_wrapper        TM_LOCK_T
#define pthread_mutex_init_wrapper(mutex,attr)      (mutex)->lock=0
#define pthread_mutex_destroy_wrapper(mutex)  /* nothing */
#define pthread_mutex_lock_wrapper(mutex)     TM_BEGIN_ARGS(mutex)
#define pthread_mutex_unlock_wrapper(mutex)    TM_END_ARGS(mutex)
#define pthread_cond_t_wrapper          /* nothing */
#define pthread_cond_init_wrapper(cond,attr)      /* nothing */
#define pthread_cond_destroy_wrapper(cond)    /* nothing */
#define pthread_cond_signal_wrapper(cond) TM_COND_SIGNAL_SPIN(cond)
#define pthread_cond_broadcast_wrapper(cond)  TM_COND_BROADCAST_SPIN(cond)
#define pthread_cond_wait_wrapper(cond, mutex)  {TM_END_ARGS(mutex);TM_BEGIN_ARGS(mutex);}
#endif

#ifdef EXTERNAL_GLOBAL_LOCK
extern TM_LOCK_T external_global_lock;
#define pthread_mutex_t_wrapper        /* nothing */
#define pthread_mutex_init_wrapper(mutex,attr)      /* nothing */
#define pthread_mutex_destroy_wrapper(mutex)  /* nothing */
#define pthread_mutex_lock_wrapper(mutex)     TM_BEGIN_ARGS(&external_global_lock)
#define pthread_mutex_unlock_wrapper(mutex)    TM_END_ARGS(&external_global_lock)
#define pthread_cond_t_wrapper          /* nothing */
#define pthread_cond_init_wrapper(cond,attr)      /* nothing */
#define pthread_cond_destroy_wrapper(cond)    /* nothing */
#define pthread_cond_signal_wrapper(cond) TM_COND_SIGNAL_SPIN(cond)
#define pthread_cond_broadcast_wrapper(cond)  TM_COND_BROADCAST_SPIN(cond)
#define pthread_cond_wait_wrapper(cond, mutex)  {TM_END_ARGS(&external_global_lock);TM_BEGIN_ARGS(&external_global_lock);}
#endif

#ifdef INTERNAL_GLOBAL_LOCK
#define pthread_mutex_t_wrapper        /* nothing */
#define pthread_mutex_init_wrapper(mutex,attr)      /* nothing */
#define pthread_mutex_destroy_wrapper(mutex)  /* nothing */
#define pthread_mutex_lock_wrapper(mutex)     TM_BEGIN()
#define pthread_mutex_unlock_wrapper(mutex)    TM_END()
#define pthread_cond_t_wrapper          /* nothing */
#define pthread_cond_init_wrapper(cond,attr)      /* nothing */
#define pthread_cond_destroy_wrapper(cond)    /* nothing */
#define pthread_cond_signal_wrapper(cond) TM_COND_SIGNAL_SPIN(cond)
#define pthread_cond_broadcast_wrapper(cond)  TM_COND_BROADCAST_SPIN(cond)
#define pthread_cond_wait_wrapper(cond, mutex)  {TM_END();TM_BEGIN();}
#endif


#else
#define pthread_mutex_t_wrapper        pthread_mutex_t
#define pthread_mutex_init_wrapper(mutex,attr)      pthread_mutex_init(mutex,attr)
#define pthread_mutex_destroy_wrapper(mutex)  pthread_mutex_destroy(mutex)
#define pthread_mutex_lock_wrapper(mutex)     pthread_mutex_lock(mutex)
#define pthread_mutex_unlock_wrapper(mutex)    pthread_mutex_unlock(mutex)
#define pthread_cond_t_wrapper          pthread_cond_t
#define pthread_cond_init_wrapper(cond,attr)      pthread_cond_init(cond,attr)
#define pthread_cond_destroy_wrapper(cond)   pthread_cond_destroy(cond)
#define pthread_cond_signal_wrapper(cond) pthread_cond_signal(cond)
#define pthread_cond_broadcast_wrapper(cond)  pthread_cond_broadcast(cond)
#define pthread_cond_wait_wrapper(cond, mutex)  pthread_cond_wait(cond, mutex)
#define TM_BEGIN() /* nothing */
#define TM_END() /* nothing */
#define TM_THREAD_ENTER() /* nothing */
#define TM_THREAD_EXIT() /* nothing */
#define TM_STARTUP(threads_num) /* nothing */
#define TM_SHUTDOWN() /* nothing */
#endif /*RTM */

#endif /*TSX_H*/
