#include "test_helpers.h"
#include "dma.h"
#include <stdio.h>
#include <unistd.h>




int main() {
    DMA_init();
    printf("MAIN 1.4 Test Débordemant table job \n");
    // TEST DE LA TAILLE 
    // ---------------------------------------------------------------------------------
    // ---------------------------------------------------------------------------------    
    int *t1_vide = clear_tab(50);
    int *t1_rempli = init_tab(50 , 0); 
    tab* src1 = create_tab((uintptr_t*)t1_rempli, 50, 4, RED_RDY);
    tab* trg1 = create_tab((uintptr_t*)t1_vide  , 50, 4, WRT_RDY);

    int *t2_vide = clear_tab(500);
    int *t2_rempli = init_tab(500 , 0); 
    tab* src2 = create_tab((uintptr_t*)t2_rempli, 500,4, RED_RDY);
    tab* trg2 = create_tab((uintptr_t*)t2_vide  , 500,4, WRT_RDY);
    
    int *t3_vide = clear_tab(200);
    int *t3_rempli = init_tab(200 , 0); 
    tab* src3 = create_tab((uintptr_t*)t3_rempli, 200,4, RED_RDY);
    tab* trg3 = create_tab((uintptr_t*)t3_vide  , 200,4, WRT_RDY);
    
    int *t4_vide = clear_tab(500);
    int *t4_rempli = init_tab(500 , 0); 
    tab* src4 = create_tab((uintptr_t*)t4_rempli, 500,4, RED_RDY);
    tab* trg4 = create_tab((uintptr_t*)t4_vide  , 500,4, WRT_RDY);
    
    int *t5_vide = clear_tab(100); 
    int *t5_rempli = init_tab(100 , 0); 
    tab* src5 = create_tab((uintptr_t*)t5_rempli, 100,4, RED_RDY);
    tab* trg5 = create_tab((uintptr_t*)t5_vide  , 100,4, WRT_RDY);

    ASSERT_LIST_LEN(r.free, MAX_JOBS);
    ASSERT_LIST_LEN(r.pending, 0);
    ASSERT_LIST_LEN(r.done, 0);

    size_t tab_c[5];
    for(size_t i=0;i<5;i++)
        tab_c[i] = i;

    // Ajout dans le tableau de vérification
    if(ajout_couple(src1, trg1, 50, &tab_c[0])) return -1;
    if(ajout_couple(src2, trg2, 500, &tab_c[0])) return -1;
    if(ajout_couple(src3, trg3, 200, &tab_c[0])) return -1;
    if(ajout_couple(src4, trg4, 500, &tab_c[0])) return -1;
    if(ajout_couple(src5, trg5, 100, &tab_c[0])) return -1;

    printf("ajout dans racine\n");
    uintptr_t j0 = DMA_job(src1, trg1, 1, 1, 50);
    uintptr_t j1 = DMA_job(src2, trg2, 1, 1, 500);
    uintptr_t j2 = DMA_job(src3, trg3, 1, 1, 200);
    uintptr_t j3 = DMA_job(src4, trg4, 1, 1, 500);

    uintptr_t j4 = 0; // Utilise un 6ème job pour vraiment tester le débordement (MAX_JOBS = 5)
    
    while(j4 == 0) {
        j4 = DMA_job(src5, trg5, 1, 1, 100);
        if(j4 == 0) {
            // Le tableau est plein, on libère le premier job s'il a fini pour faire de la place !
            DMA_done(j0); 
            usleep(1000); // Petite pause pour ne pas griller le CPU avec une boucle active
        }
    }

    printf("\n\n\n\nje suis sorti\n");
    while(!DMA_done(j1)) sleep(1);
    ASSERT_DMA_COUNT();
    printf("\n\n\n\nje suis sorti\n");
    while(!DMA_done(j2)) sleep(1);
    ASSERT_DMA_COUNT();
    printf("\n\n\n\nje suis sorti\n");

    while(!DMA_done(j3)) sleep(1);
    ASSERT_DMA_COUNT();
    while(!DMA_done(j4)); 
    ASSERT_DMA_COUNT();
    printf("\n\n\n\nje suis sorti final \n");

    printf("free %zu pending %zu done %zu\n", dt_list_size(r.free), dt_list_size(r.pending), dt_list_size(r.done));

    pthread_t verif_finale;
    if (pthread_create(&verif_finale, NULL, tab_verif_finale, NULL)){     
        fprintf(stderr, "Verif thread could not be created\n");
        return -1;
    }
    ASSERT_LIST_LEN(r.free, MAX_JOBS);
    ASSERT_DMA_COUNT();
    pthread_join(verif_finale, NULL);


    
    return 0;
}

