#include<stdio.h>
#include<pthread.h>

#include"dma.h"
#include"dma_sim.h"

#define N 10

void handle_sigint(int sig){
  running = 0;
  DMA_cleanup();

  signal(SIGINT, SIG_DFL);
  raise(SIGINT);
}

void * task(tab* t) {
  int arr1[N];
  for(int i = 0; i < N; i++) {
    arr1[i] = i;
    printf("arr1[%d] = %d\n", i, arr1[i]);
  }

  tab* tab1 = create_tab(arr1, N, (RED_RDY | WRD_CFG));

  uintptr_t job_id = DMA_job(t, tab1, 0, 0, N);
  show_jobs();

  while(!DMA_done(job_id)) {
    sleep(1);
  }
}

int main(int argc, char* argv[]){

  // Interrompre thread de la DMA
  struct sigaction sa;
  sa.sa_handler = &handle_sigint;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  sigaction(SIGINT, &sa, NULL);

  DMA_init();

  if (pthread_create(&proc, NULL, task, NULL)){     
    fprintf(stderr, "Processor thread could not be created\n");
    return -1;
  }
  
  return 0;
}
