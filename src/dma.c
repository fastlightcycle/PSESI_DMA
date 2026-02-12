#include"dma.h"
#include"dma_sim.h"
#include<stdio.h>
#include<pthread.h>

pthread_mutex_t mutex_free = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_pending = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_processing = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_done = PTHREAD_MUTEX_INITIALIZER;

job jobs[MAX_JOBS];
racine r;
pthread_t preneur_job;    

/* 
 * Initialisation de la DMA
 * Appelé lors du boot
 */
int DMA_init() {
  
  r.pending = NULL;
  r.processing = NULL;
  r.done = NULL;
  
  // Toutes les cases du tableau de jobs sont vides au début
  job* j = &jobs[0];

  r.free = j;
  j->prec = NULL;
  for(int i = 1; i < MAX_JOBS; i++) {
    j->status = FREE;
    j->next = &(jobs[i]);
    j = j->next;
    j->prec = &jobs[i-1];
  }
  j->next = NULL;

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
 * Ajout d'un job
 * Exécuté par l'OS après le syscall de DMA_job
 */
uintptr_t DMA_job(tab* src, tab* trg, int src_stp, int trg_stp, size_t size) {
  if (size != src->size || size != trg->size) {
    fprintf(stderr, "ERROR: sizes do not match\n");
    return 0;
  }

  pthread_mutex_lock(&mutex_free);
  job* j = r.free;
  if (!j) {
    fprintf(stderr, "WARNING: Job list is full!\n");
    pthread_mutex_unlock(&mutex_free);
    return 0;
  }

  if(remove_job(j, &(r.free))) {
    fprintf(stderr, "ERROR: could not remove job from free list\n");
    pthread_mutex_unlock(&mutex_free);
    return 0;
  }
  pthread_mutex_unlock(&mutex_free);

  j->status = PENDING;
  j->src = src;
  j->trg = trg;
  j->src_stp = src_stp;
  j->trg_stp = trg_stp;
  j->size = size;

  pthread_mutex_lock(&mutex_pending);
  if(add_job(j, &(r.pending))) {
    fprintf(stderr, "ERROR: could not add job to pending list\n");
    pthread_mutex_unlock(&mutex_pending);
    return 0;
  }
  pthread_mutex_unlock(&mutex_pending);

  return (uintptr_t)j;
}

/* 
 * Vérification de la fin d'un job et libération de ce job
 * Exécuté par l'OS après le syscall de DMA_done
 * Non bloquante
 */
uintptr_t DMA_done(uintptr_t id) {
  job* j = (job*)id;

  if(j->status == DONE) {
    pthread_mutex_lock(&mutex_done);
    if(remove_job(j, &(r.done))) {
      fprintf(stderr, "ERROR: could not remove job from done list\n");
      pthread_mutex_unlock(&mutex_done);
      return 0;
    }
    pthread_mutex_unlock(&mutex_done);
    
    j->status = FREE;

    pthread_mutex_lock(&mutex_free);
    if(add_job(j, &(r.free))) {
      fprintf(stderr, "ERROR: could not add job to free list\n");
      pthread_mutex_unlock(&mutex_free);
      return 0;
    }
    pthread_mutex_unlock(&mutex_free);
    
    return (uintptr_t)j;
  }

  fprintf(stderr, "WARNING: Job not found in DMA_done\n");
  return 0;
}

/*
 * Créé structure qui associe tableau à son statut
 */
tab* create_tab(int* array, size_t size, int flag) {
  if(!array) {
    fprintf(stderr, "ERROR: NULL pointer passed to create_tab()\n");
    return NULL;
  }
 
  tab* t = malloc(sizeof(tab));
  t->stt = (uintptr_t)array;
  t->end = (uintptr_t)array+size;
  t->sc_flag = flag;
  t->size = size;

  return t;
}

/*
 * Ajoute le job en tete de la liste
 * Retourne 0 si réussi, -1 sinon
 */
int add_job(job* j, job **list) {
  if(!j || !list) {
    return -1;
  }

  if(*list) {
    (*list)->prec = j;
  }
  j->next = *list;
  *list = j;

  return 0;
}

/*
 * Retire job de la liste
 * Retourne 0 si réussi, -1 sinon
 */
int remove_job(job* j, job **list) {
  if(!j || !list) {
    return -1;
  }

  if(*list == j) {
    *list = j->next;
  } else {
    (j->prec)->next = j->next;
  }

  if(j->next) {
    (j->next)->prec = j->prec;
  }

  return 0;
}

/*
 * Affiche l'état des 4 listes de jobs sur le terminal
 */
void show_jobs(){
  printf("Jobs status:\n");
  job* j;

  printf("\tFree jobs:");
  pthread_mutex_lock(&mutex_free);
  j = r.free;
  while(j) {
    printf(" [%ld]", (uintptr_t)j);
    j = j->next;
  }
  pthread_mutex_unlock(&mutex_free);
  printf("\n");

  printf("\tPending jobs:");
  pthread_mutex_lock(&mutex_pending);
  j = r.pending;
  while(j) {
    printf(" [%ld]", (uintptr_t)j);
    j = j->next;
  }
  pthread_mutex_unlock(&mutex_pending);
  printf("\n");

  printf("\tProcessing jobs:");
  pthread_mutex_lock(&mutex_processing);
  j = r.processing;
  while(j) {
    printf(" [%ld]", (uintptr_t)j);
    j = j->next;
  }
  pthread_mutex_unlock(&mutex_processing);
  printf("\n");

  printf("\tDone jobs:");
  pthread_mutex_lock(&mutex_done);
  j = r.done;
  while(j) {
    printf(" [%ld]", (uintptr_t)j);
    j = j->next;
  }
  pthread_mutex_unlock(&mutex_done);
  printf("\n");
}
