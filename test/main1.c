#include "test_helpers.h"
#include "dma.h"
#include <stdio.h>




int main() {
    DMA_init();
    // dma_tes
    // TEST DE LA TAILLE 
    // ---------------------------------------------------------------------------------
    // ---------------------------------------------------------------------------------
    printf("Init tab\n");
    
    int *t1_vide = clear_tab(50);
    int *t1_rempli = init_tab(50 , 0); 
    tab* src1 = create_tab(t1_rempli, 50, RED_RDY);
    tab* trg1 = create_tab(t1_vide  , 50, WRT_RDY);

    int *t2_vide = clear_tab(500);
    int *t2_rempli = init_tab(500 , 0); 
    tab* src2 = create_tab(t2_rempli, 500, RED_RDY);
    tab* trg2 = create_tab(t2_vide  , 500, WRT_RDY);
    
    int *t3_vide = clear_tab(2000);
    int *t3_rempli = init_tab(2000 , 0); 
    tab* src3 = create_tab(t3_rempli, 2000, RED_RDY);
    tab* trg3 = create_tab(t3_vide  , 2000, WRT_RDY);
    
    int *t4_vide = clear_tab(5000);
    int *t4_rempli = init_tab(5000 , 0); 
    tab* src4 = create_tab(t4_rempli, 5000, RED_RDY);
    tab* trg4 = create_tab(t4_vide  , 5000, WRT_RDY);
    
    int *t5_vide = clear_tab(10000); 
    int *t5_rempli = init_tab(10000 , 0); 
    tab* src5 = create_tab(t5_rempli, 10000, RED_RDY);
    tab* trg5 = create_tab(t5_vide  , 10000, WRT_RDY);

    ASSERT_LIST_LEN(r.free, MAX_JOBS);
    ASSERT_LIST_LEN(r.pending, 0);
    ASSERT_LIST_LEN(r.done, 0);

    if(ajout_couple(src1, trg1, 50, 0)){
        fprintf(stderr, "ERROR : ajout couple non réussi \n");
        return -1;
    }   
    if(ajout_couple(src2, trg2, 500, 1)){
        fprintf(stderr, "ERROR : ajout couple non réussi \n");
        return -1;
    }
    if(ajout_couple(src3, trg3, 2000, 2)){
        fprintf(stderr, "ERROR : ajout couple non réussi \n");
        return -1;
    }
    if(ajout_couple(src4, trg4, 5000, 3)){
        fprintf(stderr, "ERROR : ajout couple non réussi \n");
        return -1;
    }
    if(ajout_couple(src5, trg5, 2000, 4)){
        fprintf(stderr, "ERROR : ajout couple non réussi \n");
        return -1;
    }
    printf("ajout dans racine\n");
    uintptr_t j0 = DMA_job(src1, trg1, 1, 1, 50);
    uintptr_t j1 = DMA_job(src2, trg2, 1, 1, 500);
    uintptr_t j2 = DMA_job(src3, trg3, 1, 1, 2000);
    uintptr_t j3 = DMA_job(src4, trg4, 1, 1, 5000);
    uintptr_t j4 = DMA_job(src5, trg5, 1, 1, 10000);
    ASSERT_LIST_LEN(r.free, 0);
    ASSERT_LIST_LEN(r.pending, 5);
    ASSERT_LIST_LEN(r.done, 0);

    
    while(!DMA_done(j0)); 
    ASSERT_DMA_COUNT();
    while(!DMA_done(j1)); 
    ASSERT_DMA_COUNT();
    while(!DMA_done(j2)); 
    ASSERT_DMA_COUNT();
    while(!DMA_done(j3)); 
    ASSERT_DMA_COUNT();
    while(!DMA_done(j4)); 
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

