#ifndef TEST_HELPERS_H
#define TEST_HELPERS_H

#include "dma.h"
#include "dma_sim.h"

#define NBTESTS 3

typedef struct s_couple_tab{
    void *src;
    void *trg;
    size_t size; // taille en octet 
   // int fini;    // dma_sim écrit 1 si traitement fini, 0 si traitement non terminée 
} couple_tab;
// la création d'un DMA_JOB implique l'insertion couple(src, trg) dans une case de liste_verif
extern couple_tab liste_verif[NBTESTS];

void* tab_verif_finale(void *);
int ajout_couple(void* src, void* trg, size_t size, size_t test_num);

#endif // TEST_HELPERS