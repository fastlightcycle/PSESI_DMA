#ifndef DMA_SIM
#define DMA_SIM

#include<stdatomic.h>
#include"dma.h"

extern volatile atomic_int running; // Flag partagé pour gérer thread DMA

job * DMA_prend();
void * DMA_sim();
int DMA_fini(job* j);

#endif // DMA_SIM
