#ifndef DMA_H
#define DMA_H

#include<stdint.h>
#include<stddef.h>
#include<stdlib.h>
#include<pthread.h>

#define MAX_JOBS 4

#define ERROR   0x0001
#define FREE    0x0010
#define PENDING 0x0100
#define DONE    0x1000

typedef struct s_job {
  struct s_job  *prec;
  struct s_job  *next;
  int            status;
  uintptr_t     *src;
  uintptr_t     *trg;
  size_t         size; // Nb octet
} job;

extern job jobs[MAX_JOBS];

typedef struct s_racine {
  job* pending;
  pthread_mutex_t free_lck;
  job* free;
  pthread_mutex_t pending_lck;
  job* done;
  pthread_mutex_t done_lck;
} racine;

extern racine r;

int DMA_init();
void DMA_cleanup();
uintptr_t DMA_job(uintptr_t *src, uintptr_t *trg, size_t len);
int DMA_done(uintptr_t id);

int   add_free(job* j);
int   add_pending(job* j);
int   add_done(job* j);
job * remove_free_head();
job * remove_pending_head();
int   remove_done(job* j);

void show_jobs();

#endif // _DMA_H
