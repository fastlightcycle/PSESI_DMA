#include"dma.h"
#include"dma_sim.h"
#include<stdio.h>
#include<unistd.h>
#include<signal.h>
#include<pthread.h>


#define N 10

void handle_sigint(int sig){
  running = 0;
  DMA_cleanup();

  signal(SIGINT, SIG_DFL);
  raise(SIGINT);
}

int main (int argc, char* argv[]) {

  // Interrompre thread de la DMA
  struct sigaction sa;
  sa.sa_handler = &handle_sigint;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  sigaction(SIGINT, &sa, NULL);

  DMA_init();
  show_jobs();

  int arr1[N], arr2[N], arr3[N];
  for(int i = 0; i < N; i++) {
    arr1[i] = i;
    printf("arr1[%d] = %d\n", i, arr1[i]);
  }

  tab* tab1 = create_tab(arr1, N, (RED_RDY | WRD_CFG));
  tab* tab2 = create_tab(arr2, N, (WRT_RDY | WRD_CFG));
  tab* tab3 = create_tab(arr3, N, (WRT_RDY | WRD_CFG));

  uintptr_t job1_id = DMA_job(tab1, tab2, 0, 0, N);
  show_jobs();

  while(!DMA_done(job1_id)) {
    sleep(1);
  }
  tab2->sc_flag = RED_RDY | WRD_CFG;
  printf("0x%05x\n", tab2->sc_flag);

  for(int i = 0; i < N; i++) {
    printf("arr1[%d] = %d  |  arr2[%d] = %d\n", i, arr1[i], i, arr2[i]);
  }
  show_jobs();
  
  uintptr_t job2_id = DMA_job(tab2, tab3, 0, 0, N);
  show_jobs();

  while(!DMA_done(job2_id)) {
    sleep(1);
  }
  for(int i = 0; i < N; i++) {
    printf("arr1[%d] = %d  |  arr2[%d] = %d |  arr3[%d] = %d\n", i, arr1[i], i, arr2[i], i, arr3[i]);
  }
  show_jobs();


  free(tab1);
  free(tab2);
  free(tab3);
  DMA_cleanup();
 
  return 0;
}
