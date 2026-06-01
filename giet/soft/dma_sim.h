#ifndef DMA_SIM
#define DMA_SIM

#include"dma.h"

int DMA_init();
job * DMA_prend();
void * DMA_sim(void* args);
int DMA_fini(job* j);

#endif // DMA_SIM
