#include "test_helpers.h"
#include "dma.h"
#include <stdio.h>




int main() {
    DMA_init();

    /*-------------------------------------------------------------------------------------
    TEST STEP 
    ---------------------------------------------------------------------------------------
    */

    // dma_tes
    // TEST DE LA TAILLE 
    // ---------------------------------------------------------------------------------
    // ---------------------------------------------------------------------------------
    printf("Init tab\n");
    
    int *t1_vide = clear_tab(50);
    int *t1_rempli = init_tab(50 , 0); 
    tab* src1 = create_tab(t1_rempli, 50, RED_RDY);
    tab* trg1 = create_tab(t1_vide  , 50, WRT_RDY);

    ASSERT_LIST_LEN(r.free, MAX_JOBS);
    ASSERT_LIST_LEN(r.pending, 0);
    ASSERT_LIST_LEN(r.done, 0);

    if(ajout_couple(src1, trg1, 50, 0)){
        fprintf(stderr, "ERROR : ajout couple non réussi \n");
        return -1;
    }   
    printf("ajout dans racine\n");
    uintptr_t j0 = DMA_job(src1, trg1, 1, 1, 50);
    ASSERT_LIST_LEN(r.free, 0);
    ASSERT_LIST_LEN(r.pending, 5);
    ASSERT_LIST_LEN(r.done, 0);

    
    while(!DMA_done(j0)); 
    ASSERT_DMA_COUNT();

    printf("free %zu pending %zu done %zu\n", dt_list_size(r.free), dt_list_size(r.pending), dt_list_size(r.done));

    pthread_t verif_finale;
    if (pthread_create(&verif_finale, NULL, tab_verif_finale, NULL)){     
        fprintf(stderr, "Verif thread could not be created\n");
        return -1;
    }
    ASSERT_LIST_LEN(r.free, 5);
    ASSERT_DMA_COUNT();
    pthread_join(verif_finale, NULL);






    
    return 0;
}

