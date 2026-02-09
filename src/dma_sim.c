#include"dma.h"
#include"dma_sim.h"
#include<stdio.h>
#include<pthread.h>

/*
 * Simulation du comportement de la DMA
 */
job* DMA_prend() {
//  pthread_mutex_lock(&mutex_pending);
  job* j = r.pending;
 
  if(j) {
    if(!remove_job(j, &(r.pending))) {
      fprintf(stderr, "ERROR: could not remove job from pending list\n");
//      pthread_mutex_unlock(&mutex_pending);
      return NULL;
    }
//    pthread_mutex_unlock(&mutex_pending);
    
//    pthread_mutex_lock(&mutex_processing);
    if(!add_job(j, &(r.processing))) {
      fprintf(stderr, "ERROR: could not add job to processing list\n");
//      pthread_mutex_unlock(&mutex_processing);
      return NULL;
    }
//    pthread_mutex_unlock(&mutex_processing);

    return j;
  }

  // Un autre DMA a peut-être pris le job
  return NULL;
}

/*
 * Simulation du travail de la DMA
 */
void DMA_sim() {
  job* j;
//  while(true) {
    j = DMA_prend();
    if(j) {
      for(int i = 0; i < j->size; i++) {
        j->trg[i] = j->src[i];
      }
  
      if(!DMA_fini(j)) {
        fprintf(stderr, "ERROR: could not finish job\n");
      }
    }
    // TODO Dormir sur adresse du 1er job de la list pending
//  }
}

/*
 * Appelé par le DMA après avoir traité un job
 */
int DMA_fini(job* j) {
//  pthread_mutex_lock(&mutex_processing);
  if(!remove_job(j, &(r.processing))) {
    fprintf(stderr, "ERROR: could not remove job from processing list\n");
//    pthread_mutex_unlock(&mutex_processing);
    return 0;
  }
//  pthread_mutex_unlock(&mutex_processing);
  
//  pthread_mutex_lock(&mutex_done);
  if(!add_job(j, &(r.done))) {
    fprintf(stderr, "ERROR: could not add job to done list\n");
//    pthread_mutex_unlock(&mutex_done);
    return 0;
  }
//  pthread_mutex_unlock(&mutex_done);

  return 1;
}
