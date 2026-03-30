#include"dma.h"
#include"dma_sim.h"
#include<stdio.h>
#include<unistd.h>
#include<pthread.h>

volatile atomic_int running = 1; // Flag partagé pour gérer thread DMA

/*
 * Simulation du comportement de la DMA
 */
job * DMA_prend() {
  pthread_mutex_lock(&mutex_pending);

  job* j = remove_pending_head();
 
  if(j) {
    pthread_mutex_unlock(&mutex_pending);
    return j;
  }

  // Un autre DMA a peut-être pris le job
  pthread_mutex_unlock(&mutex_pending);
  return NULL;
}

/*
 * Simulation du travail de la DMA
 */
 
void * DMA_sim(void* args) {
  printf("DMA lancée\n");

  job* j;
  while(running) {
    j = DMA_prend();
    if(j) {
      int* src = (int*)(j->src->stt);
      int* trg = (int*)(j->trg->stt);
      for(int i = 0; i < j->size; i++) {
        trg[i] = src[i];
      }
  
      if(DMA_fini(j)) {
        fprintf(stderr, "ERROR: could not finish job, stoppong simulation\n");
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

  pthread_mutex_lock(&mutex_done);
  if(add_done(j)) {
    fprintf(stderr, "ERROR: could not add job to done list\n");
    pthread_mutex_unlock(&mutex_done);
    return -1;
  }
  pthread_mutex_unlock(&mutex_done);

  return 0;
}
