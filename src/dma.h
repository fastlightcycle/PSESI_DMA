#ifndef DMA_H
#define DMA_H

#include<stdint.h>
#include<stddef.h>
#include<stdlib.h>
#include<pthread.h>

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

extern pthread_mutex_t mutex_free;
extern pthread_mutex_t mutex_pending;
extern pthread_mutex_t mutex_done;

typedef struct s_tab {
  uintptr_t stt;
  uintptr_t end;
  int sc_flag; // Status et config
  size_t size;
} tab;

typedef struct s_job {
  struct s_job *prec;
  struct s_job *next;
  int status;
  tab* src;
  tab* trg;
  int src_stp; // Steps pour faire rotations
  int trg_stp;
  size_t size; // Nb elements
} job;

extern job jobs[MAX_JOBS];

typedef struct s_racine {
  job* pending;
  job* free;
  job* done;
} racine;

extern racine r;

tab* create_tab(int* array, size_t size, int flag);

int add_free(job* j);
job * remove_free_head();
int add_pending(job* j);
job * remove_pending_head();
int add_done(job* j);
int remove_done(job* j);

int DMA_init();
void DMA_cleanup();
uintptr_t DMA_job(tab* src, tab* trg, int src_stp, int trg_stp, size_t size);
int DMA_done(uintptr_t id);

void show_jobs();

#endif // _DMA_H
