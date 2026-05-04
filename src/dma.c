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
 * Ajout d'un job
 * Exécuté par l'OS après le syscall de DMA_job
 */
uintptr_t DMA_job(tab* src, tab* trg, int src_stp, int trg_stp, size_t size) {
  if (size != src->size || size != trg->size) {
    fprintf(stderr, "ERROR: sizes do not match\n");
    return -1;
  }

  job* j = remove_free_head();
  if (!j) {
    fprintf(stderr, "WARNING: Job list is full!\n");
    return 0;
  }

  j->status = PENDING;
  j->src = src;
  j->trg = trg;
  j->src_stp = src_stp;
  j->trg_stp = trg_stp;
  j->size = size;

  if(add_pending(j)) {
    fprintf(stderr, "ERROR: could not add job to pending list\n");
    return -1;
  }

  return (uintptr_t)j;
}

/* 
 * Vérification de la fin d'un job et libération de ce job
 * Exécuté par l'OS après le syscall de DMA_done
 * Non bloquante
 */
int DMA_done(uintptr_t id) {
  job* j = (job*)id;

  if(j->status == DONE) {
    if(remove_done(j)) {
      fprintf(stderr, "ERROR: could not remove job from done list\n");
      return -1;
    }
    
    j->status = FREE;

    if(add_free(j)) {
      fprintf(stderr, "ERROR: could not add job to free list\n");
      return -1;
    }
    
    return 1;
  }

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
 * Ajoute le job en tete de la liste free
 * Retourne 0 si réussi, -1 sinon
 */
int add_free(job* j) {
  if(!j) {
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

  return j;
}

/*
 * Ajout d'un job dans la liste done
 * Ajout en queue dans une liste circulaire simplement chaînée
 */
int add_pending(job* j) {
  if(!j) {
    return -1;
  }

  pthread_mutex_lock(&r.pending_lck);

  if(r.pending) {
    ((r.pending)->prec)->next = j;
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
  if(!j) {
    return -1;
  }

  pthread_mutex_lock(&r.done_lck);

  if(r.done) {
    (r.done)->prec = j;
  }
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
