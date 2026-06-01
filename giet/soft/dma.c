#include"dma.h"
#include "stdio.h"

job jobs[MAX_JOBS];

racine r = {
  .pending = NULL,
  .free_lck = 0,
  .free = NULL,
  .pending_lck = 0,
  .done = NULL,
  .done_lck = 0
};

static void lock_acquire(int * lock) {
    unsigned int delay = rand() >> 8;
    asm volatile (
            "lock_try:              \n"
            "ll   $2,    0(%0)      \n" /* $2 <= lock current value */
            "bnez $2,    lock_delay \n" /* retry if lock already taken */
            "li   $3,    1          \n" /* $3 <= argument for sc */
            "sc   $3,    0(%0)      \n" /* try to get lock */
            "bnez $3,    lock_ok    \n" /* exit if atomic */
            "lock_delay:            \n"
            "move $4,    %1         \n" /* $4 <= delay */
            "lock_loop:             \n"
            "addi $4,    $4,  -1    \n" /* $4 <= $4 - 1 */
            "beqz $4,    lock_loop  \n" /* test end delay */
            "nop                    \n"
            "j           lock_try   \n" /* retry if not atomic */
            "nop                    \n"
            "lock_ok:               \n"
            :
            :"r"(lock), "r"(delay)
            :"$2", "$3", "$4");
}


static void lock_release(int * lock) {
    *lock = 0;
}

/* 
 * Ajout d'un job
 * Exécuté par l'OS après le syscall de DMA_job
 */
void * DMA_job(int* src, int* trg, size_t size) {
  job* j = remove_free_head();
  if (!j) {
    return NULL_SUBMIT; // Il n'y a plus de cases vides
  }

  j->status = PENDING;
  j->src = src;
  j->trg = trg;
  j->size = size;

  if(add_pending(j)) {
    tty_printf("ERROR: could not add job to pending list\n");
    return ERROR_SUBMIT;
  }

  return (void *)j;
}

/* 
 * Vérification de la fin d'un job et libération de ce job
 * Exécuté par l'OS après le syscall de DMA_done
 * Non bloquante
 */
int DMA_done(int id) {
  job* j = (job*)id;

  if(j->status == DONE) {
    if(remove_done(j)) {
      tty_printf("ERROR: could not remove job from done list\n");
      return -1;
    }
    
    j->status = FREE;

    if(add_free(j)) {
      tty_printf("ERROR: could not add job to free list\n");
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
    return -1;
  }

  lock_acquire(&r.free_lck);

  j->prec = NULL;
  j->next = r.free;
  r.free = j;

  lock_release(&r.free_lck);

  return 0;
}

/*
 * Retire job en tete de la liste free
 */
job * remove_free_head() {
  lock_acquire(&r.free_lck);

  job * j = r.free;

  if(j) {
    r.free = j->next;
  }

  lock_release(&r.free_lck);

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

  lock_acquire(&r.pending_lck);

  if(r.pending) {
    ((r.pending)->prec)->next = j;
    (r.pending)->prec = j;
  } else {
    r.pending = j;
    j->prec = j;
  }
  j->next = NULL;

  lock_release(&r.pending_lck);

  return 0;
}

/*
 * Retire job en tete de la liste pending
 */
job * remove_pending_head() {
  lock_acquire(&r.pending_lck);

  job* j = r.pending;
  
  if(j) {
    if(j->next) {
      (j->next)->prec = j->prec;
    }
    r.pending = j->next;
  }

  lock_release(&r.pending_lck);

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

  lock_acquire(&r.done_lck);

  if(r.done) {
    (r.done)->prec = j;
  }
  j->next = r.done;
  r.done = j;

  lock_release(&r.done_lck);

  return 0;
}

/*
 * Retire job de la liste done
 */
int remove_done(job* j) {
  if(!j) {
    return -1;
  }

  lock_acquire(&r.done_lck);

  if(r.done == j) {
    r.done = j->next;
  } else {
    (j->prec)->next = j->next;
  }

  if(j->next) {
    (j->next)->prec = j->prec;
  }

  lock_release(&r.done_lck);

  return 0;
}

/*
 * Affiche l'état des 3 listes de jobs sur le terminal
 */
void show_jobs(){
  tty_printf("Racine address: %x\n", &r);

  tty_printf("Jobs status:\n");
  job* j;

  tty_printf("\tFree jobs:");
  lock_acquire(&r.free_lck);
  j = r.free;
  while(j) {
    tty_printf(" [%d]", (int)j);
    j = j->next;
  }
  lock_release(&r.free_lck);
  tty_printf("\n");

  tty_printf("\tPending jobs:");
  lock_acquire(&r.pending_lck);
  j = r.pending;
  while(j) {
    tty_printf(" [%d]", (int)j);
    j = j->next;
  }
  lock_release(&r.pending_lck);
  tty_printf("\n");

  tty_printf("\tDone jobs:");
  lock_acquire(&r.done_lck);
  j = r.done;
  while(j) {
    tty_printf(" [%d]", (int)j);
    j = j->next;
  }
  lock_release(&r.done_lck);
  tty_printf("\n");
}
