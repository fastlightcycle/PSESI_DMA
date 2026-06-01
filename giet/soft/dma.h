#ifndef DMA_H
#define DMA_H

#include<stddef.h>

#define MAX_JOBS 10

#define RED_RDY 0x00001
#define WRT_RDY 0x00010
#define WRD_CFG 0x00100
#define HLF_CFG 0x01000
#define BYT_CFG 0x10000

#define ERROR   0x0001
#define FREE    0x0010
#define PENDING 0x0100
#define DONE    0x1000

#define ERROR_SUBMIT ((void*)-1)
#define NULL_SUBMIT  ((void*)0)
#define OK           ((void*)1)

typedef struct s_job {
  struct s_job *prec;
  struct s_job *next;
  int status;
  int* src;
  int* trg;
  size_t size; // Nb elements
} job;

extern job jobs[MAX_JOBS];

typedef struct s_racine {
  job* pending;
  int free_lck;
  job* free;
  int pending_lck;
  job* done;
  int done_lck;
} racine;

extern racine r;

int add_free(job* j);
job * remove_free_head();
int add_pending(job* j);
job * remove_pending_head();
int add_done(job* j);
int remove_done(job* j);

void * DMA_job(int* src, int* trg, size_t size);
int DMA_done(int id);

void show_jobs();

#endif // _DMA_H
