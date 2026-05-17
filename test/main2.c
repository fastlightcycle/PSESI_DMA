#include "test_helpers.h"
#include "dma.h"
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

uintptr_t tab_job[5];

void* thread_create_jobs1(void* arg) {
for(int i = 0; i <= 2; i++) {
    tab_job[i] = DMA_job(liste_verif[i].src, liste_verif[i].trg, 1, 1, liste_verif[i].size);
}

    return NULL;
}

void* thread_create_jobs2(void* arg) {
for(int i = 3; i <= 4; i++) {
    tab_job[i] = DMA_job(liste_verif[i].src, liste_verif[i].trg, 1, 1, liste_verif[i].size);
}
    return NULL;
}


int main() {
    DMA_init();
    printf("MAIN 2 \n");
    int *t1_vide = clear_tab(50);
    int *t1_rempli = init_tab(50 , 0); 
    tab* src1 = create_tab(t1_rempli, 50,4, RED_RDY);
    tab* trg1 = create_tab(t1_vide  , 50,4, WRT_RDY);

    int *t2_vide = clear_tab(500);
    int *t2_rempli = init_tab(500 , 0); 
    tab* src2 = create_tab(t2_rempli, 500,4, RED_RDY);
    tab* trg2 = create_tab(t2_vide  , 500,4, WRT_RDY);
    
    int *t3_vide = clear_tab(2000);
    int *t3_rempli = init_tab(2000 , 0); 
    tab* src3 = create_tab(t3_rempli, 2000,4, RED_RDY);
    tab* trg3 = create_tab(t3_vide  , 2000,4, WRT_RDY);

    int *t4_vide = clear_tab(5000);
    int *t4_rempli = init_tab(5000 , 0); 
    tab* src4 = create_tab(t4_rempli, 5000,4, RED_RDY);
    tab* trg4 = create_tab(t4_vide  , 5000,4, WRT_RDY);
    
    int *t5_vide =   clear_tab(10000); 
    int *t5_rempli = init_tab (10000 , 0); 
    tab* src5 = create_tab(t5_rempli, 10000,4, RED_RDY);
    tab* trg5 = create_tab(t5_vide  , 10000,4, WRT_RDY);

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
    if(ajout_couple(src5, trg5, 10000, 4)){
        fprintf(stderr, "ERROR : ajout couple non réussi \n");
        return -1;
    }

    pthread_t t1, t2;
    pthread_create(&t1, NULL, thread_create_jobs1, NULL);
    pthread_create(&t2, NULL, thread_create_jobs2, NULL);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);


    printf("affiche tab\n\n");
    for(int i=0;i<5;i++){
        printf("%zu\n",((job*)tab_job[i])->size);
    }








    printf("%zu %zu %zu\n\n ",dt_list_size(r.free), dt_list_size(r.pending), dt_list_size(r.done)); 
    sleep(1);
    printf("%zu %zu %zu\n\n ",dt_list_size(r.free), dt_list_size(r.pending), dt_list_size(r.done));
    
    while(!DMA_done(tab_job[0])); 
    ASSERT_DMA_COUNT();
    while(!DMA_done(tab_job[1])); 
    ASSERT_DMA_COUNT();
    while(!DMA_done(tab_job[2])); 
    ASSERT_DMA_COUNT();
    while(!DMA_done(tab_job[3])); 
    ASSERT_DMA_COUNT();
    while(!DMA_done(tab_job[4])); 
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