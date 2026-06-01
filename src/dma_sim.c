#include"dma.h"
#include"dma_sim.h"
#include<stdio.h>
#include<unistd.h>
#include<pthread.h>
#include<string.h>

volatile atomic_int running = 1; // Flag partagé pour gérer thread DMA

/*
 * Simulation du travail de la DMA
 */
void * DMA_sim(void* args) {
  printf("-------------------\nDMA lancée\n-------------------\n");

  job* j;
  while(running) {
    j = remove_pending_head();
    if(j) {
      memcpy(j->trg, j->src, j->size);
  
      if(DMA_fini(j)) {
        fprintf(stderr, "DMA_SIM: could not finish job, stopping simulation\n");
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
  if(add_done(j)) {
    fprintf(stderr, "DMA_FINI: could not add job to done list\n");
    return -1;
  }
  j->status = DONE;

  return 0;
}
