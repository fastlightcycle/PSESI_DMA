#include"dma.h"
#include"dma_sim.h"
#include "stdio.h"

/* 
 * Initialisation de la DMA
 * Appelé lors du boot
 */
int DMA_init() {

  // Toutes les cases du tableau de jobs sont vides au début
  for(int i = 0; i < MAX_JOBS; i++) {
    (jobs[i]).status = FREE;
    (jobs[i]).prec = NULL;
    (jobs[i]).next = NULL;
    if (add_free(&(jobs[i]))) {
	    return 1;
    }
  }

  return 0; 
}

/*
 * Simulation du travail de la DMA
 */
void * DMA_sim(void* args) {
  tty_printf("DMA lancée\n");

  int delay = 0;

  job* j;
  while(1) {
    delay++;
    delay = delay%100000;

    if(delay == 0) {
  	    tty_printf("DMA lit la liste pending\n");
	    j = remove_pending_head();
	    if(j) {
	      tty_printf("DMA traite le job %d\n",(int)j);
	      show_jobs();

	      int* src = j->src;
	      int* trg = j->trg;
	      for(int i = 0; i < j->size; i++) {
		trg[i] = src[i];
	      }
	  
	      if(DMA_fini(j)) {
		tty_printf("ERROR: could not finish job, stopping simulation\n");
		return NULL;
	      }
	    }
	  }
  }
  return NULL;
}

/*
 * Appelé par le DMA après avoir traité un job
 */
int DMA_fini(job* j) {
  j->status = DONE;

  if(add_done(j)) {
    tty_printf("ERROR: could not add job to done list\n");
    return -1;
  }

  return 0;
}
