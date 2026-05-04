#include"dma.h"
#include"dma_sim.h"
#include<stdio.h>
#include<unistd.h>
#include<pthread.h>

volatile atomic_int running = 1; // Flag partagé pour gérer thread DMA

/*
 * Simulation du travail de la DMA
 */
void * DMA_sim(void* args) {
  printf("DMA lancée\n");

  job* j;
  while(running) {
    j = remove_pending_head();
    if(j) {
      int* src = (int*)(j->src->stt);
      int* trg = (int*)(j->trg->stt);
      for(int i = 0; i < j->size; i++) {
        trg[i] = src[i];
      }
  
      if(DMA_fini(j)) {
        fprintf(stderr, "ERROR: could not finish job, stopping simulation\n");
        DMA_cleanup();
      }
    }

    sleep(1);
  }
  return NULL;
}

/*
 * Appelé par le DMA après avoir traité un job
 */
int DMA_fini(job* j) {
  j->status = DONE;

  if(add_done(j)) {
    fprintf(stderr, "ERROR: could not add job to done list\n");
    return -1;
  }

  return 0;
}
