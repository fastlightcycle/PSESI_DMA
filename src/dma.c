#include"dma.h"
#include"dma_sim.h"
#include<stdio.h>
#include<pthread.h>

job jobs[MAX_JOBS];

racine r = {
  .pending = NULL,
  .free_lck = PTHREAD_MUTEX_INITIALIZER,
  .free = NULL,
  .pending_lck = PTHREAD_MUTEX_INITIALIZER,
  .done = NULL,
  .done_lck = PTHREAD_MUTEX_INITIALIZER
};
  
pthread_t preneur_job;    

/* 
 * Initialisation de la DMA
 * Appelé lors du boot
 */
int DMA_init() {

  // Toutes les cases du tableau de jobs sont vides au début
  for(int i = 0; i < MAX_JOBS; i++) {
    (jobs[i]).status = FREE;
    add_free(&(jobs[i]));
  }

  if (pthread_create(&preneur_job, NULL, DMA_sim, NULL)){     
    fprintf(stderr, "DMA thread could not be created\n");
    return -1;
  }    
  
  return 0; 
}

/*
 * Réception du thread de la DMA
 * Appeler à la toute fin de tests
 */
void DMA_cleanup() {
  running = 0;
  pthread_join(preneur_job, NULL);   
}

/* 
 * Initialisation d'un job et ajout de ce job dans la liste pending
 *
 * Exécuté par l'OS après le syscall de DMA_job
 *
 * Valeurs de retour:
 *      @ job: Succès
 *      0    : Liste free est vide - il n'y a pas de place pour un nouveau job
 *      -1   : Erreur
 */
uintptr_t DMA_job(uintptr_t* src, uintptr_t* trg, size_t size) {
  if(!src){
    fprintf(stderr, "DMA_JOB: src address is invalid\n");
    return -1;
  }

  if(!trg){
    fprintf(stderr, "DMA_JOB: src address is invalid\n");
    return -1;
  }

  job* j = remove_free_head();
  if (!j) {
    return 0;
  }

  j->status = PENDING;
  j->src = src;
  j->trg = trg;
  j->size = size;

  if(add_pending(j)) {
    fprintf(stderr, "DMA_JOB: could not add job to pending list\n");
    return -1;
  }

  return (uintptr_t)j;
}

/* 
 * Vérification de la fin d'un job et libération de ce job
 *
 * Exécuté par l'OS après le syscall de DMA_done
 * Non bloquante
 *
 * Valeurs de retour:
 *      1  : Succès
 *      0  : Job en cours de traitement
 *      -1 : Erreur
 */
int DMA_done(uintptr_t id) {
  if(id < (uintptr_t)&jobs[0] || id > (uintptr_t)&jobs[MAX_JOBS-1]){
    fprintf(stderr, "DMA_DONE: Job address is out of bounds\n");
    return -1;
  }

  job* j = (job*)id;

  if(j->status == FREE){
    fprintf(stderr, "DMA_DONE: Job's status is FREE, maybe it has already been acknowledged?\n");
    return -1;
  }

  if(j->status == DONE) {
    if(remove_done(j)) {
      fprintf(stderr, "DMA_DONE: could not remove job from done list\n");
      return -1;
    }
    
    j->status = FREE;

    if(add_free(j)) {
      fprintf(stderr, "DMA_DONE: could not add job to free list\n");
      return -1;
    }
    
    return 1;
  }

  return 0;
}

/*
 * Ajoute le job en tete de la liste free
 * Retourne 0 si réussi, -1 sinon
 */
int add_free(job* j) {
  if(!j) {
    fprintf(stderr, "ADD_FREE: Bad job address\n");
    return -1;
  }
  if (j->status == PENDING){
    fprintf(stderr, "ADD_FREE: Job is in PENDING list, it must pass through DONE\n");
    return -1;
  }

  pthread_mutex_lock(&r.free_lck);

  j->prec = NULL;
  j->next = r.free;
  r.free = j;

  pthread_mutex_unlock(&r.free_lck);

  return 0;
}

/*
 * Retire job en tete de la liste free
 */
job * remove_free_head() {
  pthread_mutex_lock(&r.free_lck);

  job * j = r.free;

  if(j) {
    r.free = j->next;
  }

  pthread_mutex_unlock(&r.free_lck);

  return j; // Retournera NULL si la liste est vide
}

/*
 * Ajout d'un job dans la liste pending
 * Ajout en queue dans une liste circulaire simplement chaînée
 */
int add_pending(job* j) {
  if(!j) {
    fprintf(stderr, "ADD_PENDING: Bad job address\n");
    return -1;
  }

  pthread_mutex_lock(&r.pending_lck);

  if(r.pending) {
    ((r.pending)->prec)->next = j;
    j->prec = (r.pending)->prec;
    (r.pending)->prec = j;
  } else {
    r.pending = j;
    j->prec = j;
  }
  j->next = NULL;

  pthread_mutex_unlock(&r.pending_lck);

  return 0;
}

/*
 * Retire job en tete de la liste pending
 */
job * remove_pending_head() {
  pthread_mutex_lock(&r.pending_lck);

  job* j = r.pending;
  
  if(j) {
    if(j->next) {
      (j->next)->prec = j->prec;
    }
    r.pending = j->next;
  }

  pthread_mutex_unlock(&r.pending_lck);

  return j;
}

/*
 * Ajout d'un job dans la liste done
 * Ajout en tete dans une liste doublement chaînée
 */
int add_done(job* j) {
  if(!j){
    fprintf(stderr, "ADD_DONE: Bad job address\n");
    return -1;
  } 

  pthread_mutex_lock(&r.done_lck);

  if(r.done) {
    (r.done)->prec = j;
  }
  j->prec = NULL;
  j->next = r.done;
  r.done = j;

  pthread_mutex_unlock(&r.done_lck);

  return 0;
}

/*
 * Retire job de la liste done
 */
int remove_done(job* j) {
  if(!j) {
    fprintf(stderr, "REMOVE_DONE: Bad job address\n");
    return -1;
  }

  pthread_mutex_lock(&r.done_lck);

  if(r.done == j) {
    r.done = j->next;
  } else {
    (j->prec)->next = j->next;
  }

  if(j->next) {
    (j->next)->prec = j->prec;
  }

  pthread_mutex_unlock(&r.done_lck);

  return 0;
}

/*
 * Affiche l'état des 3 listes de jobs sur le terminal
 * Pour le débuggage
 */
void show_jobs(){
  printf("Jobs status:\n");
  job* j;

  printf("\tFree jobs:");
  pthread_mutex_lock(&r.free_lck);
  j = r.free;
  while(j) {
    printf(" [%ld]", (uintptr_t)j);
    j = j->next;
  }
  pthread_mutex_unlock(&r.free_lck);
  printf("\n");

  printf("\tPending jobs:");
  pthread_mutex_lock(&r.pending_lck);
  j = r.pending;
  while(j) {
    printf(" [%ld]", (uintptr_t)j);
    j = j->next;
  }
  pthread_mutex_unlock(&r.pending_lck);
  printf("\n");

  printf("\tDone jobs:");
  pthread_mutex_lock(&r.done_lck);
  j = r.done;
  while(j) {
    printf(" [%ld]", (uintptr_t)j);
    j = j->next;
  }
  pthread_mutex_unlock(&r.done_lck);
  printf("\n");
}
