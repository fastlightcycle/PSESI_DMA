#ifndef DMA_H
#define DMA_H

#include<stdint.h>
#include<stddef.h>
#include<stdlib.h>
#include<pthread.h>

#define MAX_JOBS 10

#define RED_RDY 0x0000
#define WRT_RDY 0x0001
#define WRD_CFG 0x0010
#define HLF_CFG 0x0100
#define BYT_CFG 0x1000

typedef struct s_tab {
  uintptr_t src_stt;
  uintptr_t src_end;
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
