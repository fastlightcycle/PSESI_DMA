#include "test_helpers.h"
#include "dma.h"
#include <stdio.h>



int main() {
    DMA_init();
    int t1[5], t2[5], t3[5], t4[5], t5[5], t6[5];
    for(int i=0; i<5;i++){
        t1[i] = i;
        t3[i] = 10 + i;
        t5[i] = 100 + i;
    }
    tab* src1 = create_tab(t1, 5, RED_RDY);
    tab* trg1 = create_tab(t2, 5, WRT_RDY);
    tab* src2 = create_tab(t3, 5, RED_RDY);
    tab* trg2 = create_tab(t4, 5, WRT_RDY);
    tab* src3 = create_tab(t5, 5, RED_RDY);
    tab* trg3 = create_tab(t6, 5, WRT_RDY);

    uintptr_t j0 = DMA_job(src1, trg1, 1, 1, 5);
    uintptr_t j1 = DMA_job(src2, trg2, 1, 1, 5);
    uintptr_t j2 = DMA_job(src3, trg3, 1, 1, 5);

    ajout_couple((void*)t1, (void*)t2, 5, 0);
    ajout_couple((void*)t3, (void*)t4, 5, 1);
    ajout_couple((void*)t5, (void*)t6, 5, 2);

    while(!DMA_done(j0));
    while(!DMA_done(j1));
    while(!DMA_done(j2));

    pthread_t verif_finale;
    if (pthread_create(&verif_finale, NULL, tab_verif_finale, NULL)){     
        fprintf(stderr, "Verif thread could not be created\n");
        return -1;
    }

    pthread_join(verif_finale, NULL);

    return 0;
}

