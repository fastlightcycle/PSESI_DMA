#include"dma.h"
#include<stdio.h>
#include<pthread.h>

pthread_mutex_t mutex_free = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_pending = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_processing = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_done = PTHREAD_MUTEX_INITIALIZER;

job jobs[MAX_JOBS];
racine r;

/* 
 * Initialisation de la DMA
 * Appelé lors du boot
 */
void DMA_init() {
  
  r.pending = NULL;
  r.processing = NULL;
  r.done = NULL;
  
  // Toutes les cases du tableau de jobs sont vides au début
  job* j = &jobs[0];

  r.free = j;
  j->id = (uintptr_t)j;
  j->prec = NULL;
  for(int i = 1; i < MAX_JOBS; i++) {
    j->next = &(jobs[i]);
    j = j->next;
    j->prec = &jobs[i-1];
    j->id = (uintptr_t)j;
  }
  j->next = NULL;

  pthread_t preneur_job;    
  if (pthread_create(&preneur_job, NULL, DMA_sim, NULL) != 0){     
    // Gestion d'erreur si le thread ne peut pas être créé     
    return 1;   
  }    
  
  pthread_join(preneur_job, NULL);   
  return 0; 
}


/* 
 * Ajout d'un job
 * Exécuté par l'OS après le syscall de DMA_job
 */
uintptr_t DMA_job(int* src, int* trg, size_t size) {
  pthread_mutex_lock(&mutex_free);
  job* j = r.free;
  if (!j) {
    printf("ERROR: Job list is full!\n");
   // pthread_mutex_unlock(&mutex_free);
    return 0;
  }

  if(!remove_job(j, &(r.free))) {
    fprintf(stderr, "ERROR: could not remove job from free list\n");
  //  pthread_mutex_unlock(&mutex_free);
    return 0;
  }
//  pthread_mutex_unlock(&mutex_free);

  j->src = src;
  j->trg = trg;
  j->size = size;

 // pthread_mutex_lock(&mutex_pending);
  if(!add_job(j, &(r.pending))) {
    fprintf(stderr, "ERROR: could not add job to pending list\n");
  //  pthread_mutex_unlock(&mutex_pending);
    return 0;
  }
  //pthread_mutex_unlock(&mutex_pending);

  return j->id;
}

/* 
 * Vérification de la fin d'un job et libération de ce job
 * Exécuté par l'OS après le syscall de DMA_done
 * Version bloquante
 */
uintptr_t DMA_done(uintptr_t id) {
  //pthread_mutex_lock(&mutex_done);
  job *j = r.done;

  while(j) {
    if (j->id == id) {
      if(!remove_job(j, &(r.done))) {
        fprintf(stderr, "ERROR: could not remove job from done list\n");
       // pthread_mutex_unlock(&mutex_done);
        return 0;
      }
 //     pthread_mutex_unlock(&mutex_done);
      
   //   pthread_mutex_lock(&mutex_free);
      if(!add_job(j, &(r.free))) {
        fprintf(stderr, "ERROR: could not add job to free list\n");
     //   pthread_mutex_unlock(&mutex_free);
        return 0;
      }
 //     pthread_mutex_unlock(&mutex_free);

      return j->id;
    }

    j = j->next;
  }

  fprintf(stderr, "WARNING: Job not found in DMA_done\n");
  return 0;
}

/*
 * Retire job de la liste
 * Retourne 1 si réussi, 0 sinon
 */
int remove_job(job* j, job **list) {
  if(!j || !list) {
    return 0;
  }

  if(*list == j) {
    *list = j->next;
  } else {
    (j->prec)->next = j->next;
  }

  if(j->next) {
    (j->next)->prec = j->prec;
  }

  return 1;
}

/*
 * Ajoute le job en tete de la liste
 * Retourne 1 si réussi, 0 sinon
 */
int add_job(job* j, job **list) {
  if(!j || !list) {
    return 0;
  }

  if(*list) {
    (*list)->prec = j;
  }
  j->next = *list;
  *list = j;

  return 1;
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
    printf(" [%ld]", j->id);
    j = j->next;
  }
 // pthread_mutex_unlock(&mutex_free);
  printf("\n");

  printf("\tPending jobs:");
  //pthread_mutex_lock(&mutex_pending);
  j = r.pending;
  while(j) {
    printf(" [%ld]", j->id);
    j = j->next;
  }
 // pthread_mutex_unlock(&mutex_pending);
  printf("\n");

  printf("\tProcessing jobs:");
 // pthread_mutex_lock(&mutex_processing);
  j = r.processing;
  while(j) {
    printf(" [%ld]", j->id);
    j = j->next;
  }
 // pthread_mutex_unlock(&mutex_processing);
  printf("\n");

  printf("\tDone jobs:");
//  pthread_mutex_lock(&mutex_done);
  j = r.done;
  while(j) {
    printf(" [%ld]", j->id);
    j = j->next;
  }
  //pthread_mutex_unlock(&mutex_done);
  printf("\n");
}
