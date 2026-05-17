#include "test_helpers.h"
#include "dma.h"
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>

#define NB_USERS 4
#define ITERATIONS 3 // Nombre de requêtes par utilisateur pour que le test se termine

void* user_thread(void* arg) {
    int user_id = *(int*)arg;
    printf("[User %d] Connecté.\n", user_id);

    for (int i = 0; i < ITERATIONS; i++) {
        // 1. Préparation (tableaux uniques par utilisateur)
        size_t len = 50; 
        int *src_data = init_tab(len, user_id * 1000 + i); // Données spécifiques
        int *trg_data = clear_tab(len);
        tab* src = create_tab((uintptr_t*)src_data, len, sizeof(int), RED_RDY);
        tab* trg = create_tab((uintptr_t*)trg_data, len, sizeof(int), WRT_RDY);

        // 2. Soumission sécurisée (attente si la file est pleine)
        uintptr_t job_id = 0;
        while (job_id == 0) {
            job_id = DMA_job(src, trg, 1, 1, len);
            if (job_id == 0) usleep(10000); // 10ms de pause avant de réessayer
        }
        printf("[User %d] Job soumis (itération %d)\n", user_id, i + 1);

        // 3. Attente active de la fin
        while (!DMA_done(job_id)) {
            usleep(5000); // 5ms de pause
        }
        printf("[User %d] Job terminé\n", user_id);

        // 4. Vérification rapide
        if (tab_verif_rapide(src, trg) != 0) {
            fprintf(stderr, "[User %d] ERREUR DE COPIE !\n", user_id);
        }

        // Nettoyage mémoire (facultatif si tes tests s'arrêtent vite)
        free(src_data); free(trg_data); free(src); free(trg);

        // 5. Attente d'1 seconde comme demandé
        sleep(1);
    }
    
    printf("[User %d] Déconnecté.\n", user_id);
    return NULL;
}

int main() {
    printf("MAIN 3 : Simulation SoC multi-utilisateurs (4 threads)\n");
    DMA_init();

    pthread_t users[NB_USERS];
    int user_ids[NB_USERS];

    // Lancement des threads
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
    printf("MAIN 3 : Succès.\n");
    return 0;
}