#include "test_helpers.h"
#include "dma.h"
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>

#define NB_USERS 4
#define JOB_SIZE 20000

void* user_thread(void* arg) {
    int user_id = *(int*)arg;
    printf("[User %d] Connecté. Préparation des données...\n", user_id);

    // 1. Préparation de DEUX gros transferts par utilisateur
    int *src_data1 = init_tab(JOB_SIZE, user_id * 1000); 
    int *trg_data1 = clear_tab(JOB_SIZE);
    tab* src1 = create_tab((uintptr_t*)src_data1, JOB_SIZE, sizeof(int), RED_RDY);
    tab* trg1 = create_tab((uintptr_t*)trg_data1, JOB_SIZE, sizeof(int), WRT_RDY);

    int *src_data2 = init_tab(JOB_SIZE, user_id * 2000); 
    int *trg_data2 = clear_tab(JOB_SIZE);
    tab* src2 = create_tab((uintptr_t*)src_data2, JOB_SIZE, sizeof(int), RED_RDY);
    tab* trg2 = create_tab((uintptr_t*)trg_data2, JOB_SIZE, sizeof(int), WRT_RDY);

    // 2. Soumission du Job 1
    uintptr_t j1 = 0;
    while (j1 == 0) {
        j1 = DMA_job(src1, trg1, 1, 1, JOB_SIZE);
        if (j1 == 0) {
            printf("[User %d] Congestion ! En attente pour le Job 1...\n", user_id);
            usleep(10000);
        }
    }
    printf("[User %d] Job 1 soumis avec succès.\n", user_id);

    // 3. Soumission du Job 2 (SANS ATTENDRE LA FIN DU JOB 1) -> Provoque la congestion
    uintptr_t j2 = 0;
    while (j2 == 0) {
        j2 = DMA_job(src2, trg2, 1, 1, JOB_SIZE);
        if (j2 == 0) {
            // C'est ici que tu vas voir les threads patienter !
            printf("[User %d] Congestion ! File pleine, en attente pour le Job 2...\n", user_id);
            usleep(10000); 
        }
    }
    printf("[User %d] Job 2 soumis avec succès.\n", user_id);

    // 4. Attente de la complétion des deux jobs
    printf("[User %d] Attend la fin de ses transferts...\n", user_id);
    while (!DMA_done(j1)) usleep(5000);
    while (!DMA_done(j2)) usleep(5000);
    printf("[User %d] Transferts terminés.\n", user_id);

    // 5. Vérification
    if (tab_verif_rapide(src1, trg1) != 0 || tab_verif_rapide(src2, trg2) != 0) {
        fprintf(stderr, "[User %d] ERREUR DE COPIE DANS LES DONNÉES !\n", user_id);
    } else {
        printf("[User %d] Vérification des données (%d éléments) : OK.\n", user_id, JOB_SIZE * 2);
    }

    // Nettoyage mémoire
    free(src_data1); free(trg_data1); free(src1); free(trg1);
    free(src_data2); free(trg_data2); free(src2); free(trg2);

    return NULL;
}

int main() {
    printf("MAIN 3 : Test de Congestion SoC (4 utilisateurs x 2 gros jobs)\n");
    printf("Taille de transfert : %d éléments par job.\n", JOB_SIZE);
    
    DMA_init();

    pthread_t users[NB_USERS];
    int user_ids[NB_USERS];

    // Lancement simultané de tous les threads
    for (int i = 0; i < NB_USERS; i++) {
        user_ids[i] = i + 1; // ID 1 à 4
        if (pthread_create(&users[i], NULL, user_thread, &user_ids[i]) != 0) {
            fprintf(stderr, "Erreur création thread %d\n", i);
            return -1;
        }
    }

    // Attente de la fin des threads
    for (int i = 0; i < NB_USERS; i++) {
        pthread_join(users[i], NULL);
    }

    DMA_cleanup();
    printf("MAIN 3 : Test de congestion terminé avec succès.\n");
    return 0;
}