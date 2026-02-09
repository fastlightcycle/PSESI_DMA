#ifndef DMA_H
#define DMA_H

#include<stdint.h>
#include<stddef.h>
#include<stdlib.h>
#include<pthread.h>

#define MAX_JOBS 10

typedef struct s_job {
  int* src;
  int* trg;
  size_t size;
  uintptr_t id;
  struct s_job *prec;
  struct s_job *next;
} job;

extern job jobs[MAX_JOBS];

typedef struct s_racine {
  job* pending;
  job* free;
  job* processing;
  job* done;
} racine;

extern racine r;

int remove_job(job* j, job **list);
int add_job(job* j, job **list);

void DMA_init();
uintptr_t DMA_job(int* src, int* trg, size_t size);
uintptr_t DMA_done(uintptr_t id);

void show_jobs();

#endif // _DMA_H
