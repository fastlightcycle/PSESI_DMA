#include <stdlib.h>
#include <stdio.h>
#include "test_helpers.h"
#include "dma.h"

int main() {
    DMA_init();
    printf("Init tab avec types multiples\n");
    
    // 1. Test INT
    size_t sz1 = 50;
    int *s1 = malloc(sz1*sizeof(int)); for(int i=0; i<50; i++) s1[i] = i;
    int *t1 = calloc(50, sizeof(int));
    tab* src1 = create_tab((uintptr_t*)s1, sz1, sizeof(int), RED_RDY);
    tab* trg1 = create_tab((uintptr_t*)t1, sz1, sizeof(int), WRT_RDY);

    // 2. Test CHAR
    size_t sz2 = 500;
    char *s2 = malloc(sz2*sizeof(char)); for(int i=0; i<500; i++) s2[i] = 'A' + (i % 26);
    char *t2 = calloc(500, sizeof(char));
    tab* src2 = create_tab((uintptr_t*)s2, sz2, sizeof(char), RED_RDY);
    tab* trg2 = create_tab((uintptr_t*)t2, sz2, sizeof(char), WRT_RDY);
    
    // 3. Test FLOAT
    size_t sz3 = 2000;

    float *s3 = malloc(2000*sizeof(float)); for(int i=0; i<2000; i++) s3[i] = i + 0.5f;
    float *t3 = calloc(2000, sizeof(float));
    tab* src3 = create_tab((uintptr_t*)s3, 2000, sizeof(float), RED_RDY);
    tab* trg3 = create_tab((uintptr_t*)t3, 2000, sizeof(float), WRT_RDY);
    
    // 4. Test DOUBLE
    size_t sz4 = 5000;
    double *s4 = malloc(sz4*sizeof(double)); for(int i=0; i<5000; i++) s4[i] = i * 3.14;
    double *t4 = calloc(5000, sizeof(double));
    tab* src4 = create_tab((uintptr_t*)s4, sz4, sizeof(double), RED_RDY);
    tab* trg4 = create_tab((uintptr_t*)t4, sz4, sizeof(double), WRT_RDY);
    
    // 5. Test SHORT
    size_t sz5 = 10000;
    short *s5 = malloc(sz5*sizeof(short)); for(int i=0; i<10000; i++) s5[i] = i % 100;
    short *t5 = calloc(10000, sizeof(short));
    tab* src5 = create_tab((uintptr_t*)s5, sz5, sizeof(short), RED_RDY);
    tab* trg5 = create_tab((uintptr_t*)t5, sz5, sizeof(short), WRT_RDY);

    ASSERT_LIST_LEN(r.free, MAX_JOBS);
    ASSERT_LIST_LEN(r.pending, 0);
    ASSERT_LIST_LEN(r.done, 0);

    size_t tab_c[5];
    for(size_t i=0;i<5;i++)
        tab_c[i] = i;
    // Ajout dans le tableau de vérification
    if(ajout_couple(src1, trg1, sz1, &tab_c[0])) return -1;
    if(ajout_couple(src2, trg2, sz2, &tab_c[0])) return -1;
    if(ajout_couple(src3, trg3, sz3, &tab_c[0])) return -1;
    if(ajout_couple(src4, trg4, sz4, &tab_c[0])) return -1;
    if(ajout_couple(src5, trg5, sz5, &tab_c[0])) return -1;

    printf("Ajout dans racine\n");
    
    // Soumission des jobs au DMA
    uintptr_t j0 = DMA_job(src1, trg1, 1, 1, sz1);
    printf("a_\n");
    uintptr_t j1 = DMA_job(src2, trg2, 1, 1, sz2);
    printf("a_\n");
    uintptr_t j2 = DMA_job(src3, trg3, 1, 1, sz3);
    printf("a_\n");
    uintptr_t j3 = DMA_job(src4, trg4, 1, 1, sz4);
    printf("a_\n");
    uintptr_t j4 = DMA_job(src5, trg5, 1, 1, sz5);
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