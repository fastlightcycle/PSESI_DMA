#include"dma.h"
#include"dma_sim.h"
#include<stdio.h>
#include<unistd.h>
#include<pthread.h>
#include <string.h>

volatile atomic_int running = 1; // Flag partagé pour gérer thread DMA

/*
 * Simulation du travail de la DMA
 */
 
void *DMA_sim() {
  printf("\n-------------------\nDMA lancée\n-------------------\n");

  job* j;
  while(running) {
    j = remove_pending_head();
    if(j) { 
      //printf("DMA_SIM : prend un nouveau job , len : %zu\n",j->size);
      // Pointeurs génériques en octets (pour avancer case par case)
      uint8_t* p_src = (uint8_t*)j->src->stt;
      uint8_t* p_trg = (uint8_t*)j->trg->stt;
      size_t s_data = j->src->size_data;
      //printf("en cours de traitement \n");
      // Optimisation : Si tout est contigu (step = 1), un seul gros memcpy
      if (j->src_stp == 1 && j->trg_stp == 1) {
          //printf("il doit entré dedans\n");
          memcpy(p_trg, p_src, j->size * s_data);
      }
      // Sinon, gestion du step
      else {
          for(size_t i = 0; i < j->size; i++) {
              // Calcul du décalage (offset) en octets
              size_t offset_src = i * j->src_stp * s_data;
              size_t offset_trg = i * j->trg_stp * s_data;

              // Copie d'UN SEUL élément à la bonne adresse
              memcpy(p_trg + offset_trg, p_src + offset_src, s_data);
          }
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

  if(add_done(j)) {
    fprintf(stderr, "ERROR: could not add job to done list\n");
    return -1;
  }
  j->status = DONE;

  return 0;
}
