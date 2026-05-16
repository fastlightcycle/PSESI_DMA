#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include "dma.h"
#include "dma_sim.h"

int count_jobs(job* list){
    int count = 0;
    job* current = list;
    while (current != NULL){
        count++;
        current = current->next;
    }
    return count;
}

int main(){
    printf("=== TEST UNITAIRE 1 : VALIDATION DU CYCLE DE VIE SÉQUENTIEL ===\n\n");  
    
    // 1. INITIALISATION
    printf("[1] Initialisation du système...\n");
    DMA_init();

    // Vérification de l'invariant de masse au démarrage
    int total_init = count_jobs(r.free);
    printf("    Jobs dans la liste FREE : %d / %d\n", total_init, MAX_JOBS);
    assert(total_init == MAX_JOBS);
    assert(r.pending == NULL);
    assert(r.done == NULL);
    printf("    OK : Système initialisé proprement.\n\n");

    // 2. CRÉATION DES TABLEAUX
    printf("[2] Création des structures de tableaux (src et trg)...\n");
    int data_src[5] = {10, 20, 30, 40, 50};
    int data_trg[5] = {0,0,0,0,0};

    tab* ts = create_tab(data_src, 5, RED_RDY | WRT_RDY);
    tab* tt = create_tab(data_trg, 5, WRT_RDY);

    assert(ts != NULL && tt != NULL);
    assert(ts->size == 5);
    printf("    OK : Tableaux créés avec succès.\n\n");
    
    // 3.  SOUMISSION D'UN JOB (DMA_job)
    printf("[3] Soumission d'un job de transfert ...\n");
    uintptr_t job_id = DMA_job(ts, tt, 1, 1, 5);

    if (job_id == 0){
        printf("    ERREUR : Échec de la création du job.\n\n");
        return -1;
    }

    // 4. ACQUISITION PAR LA DMA (DMA_prend)
    printf("[4] Simulation de l'acquisition matérielle (DMA_prend)...\n");
    job* j_taken = DMA_prend(); // de base il doit être fait par dma_sim , pour cette fois , test atomique 

    assert(j_taken != NULL);
    assert((uintptr_t)j_taken == job_id);

    // Vérification du verrouillage des flags
    assert(!(ts->sc_flag & WRT_RDY)); // Source ne doit plus être modifiable
    assert(!(tt->sc_flag & RED_RDY)); // Cible ne doit pas être lisible
    assert(!(tt->sc_flag & WRT_RDY)); // Cible ne doit pas être modifiable

    assert(count_jobs(r.pending) == 0);
    printf("    OK : Job acquis eet ressources verrouillées.\n\n");

    // 5. TRAVAIL DE COPIE (Simulé)
    printf("[5] Exécution de la copie mémoire...\n");
    int* s_ptr = (int*)j_taken->src->stt;
    int* t_ptr = (int*)j_taken->trg->stt;
    for(int i = 0; i < j_taken->size; i++){
        t_ptr[i] = s_ptr[i];
    }
    printf("    Données copiées : %d %d %d %d %d\n", data_trg[0], data_trg[1], data_trg[2], data_trg[3], data_trg[4], data_trg[5]);
    printf("    OK : Transfert terminé.\n\n");

    // 6. FINALISATION MATÉRIELLE (DMA_fini)
    printf("[6] Notification de fin de transfert (DMA_fini)...\n");
    int res_fini = DMA_fini(j_taken);

    assert(res_fini == 1);
    assert(j_taken->status == DONE);
    assert(count_jobs(r.done) == 1);

    // Vérification de la restitution des flags 
    assert(tt->sc_flag & RED_RDY); // Cible est maintenant prête pour l'utilisateur 
    printf("    OK : Job déplacé vers la liste DONE, flags restitués.\n\n");

    // 7. LIBÉRATION PAR L'UTILISATEUR (DMA_done)
    printf("[7] Libération du job par le thread utilisateur (DMA_done)...\n");
    uintptr_t res_done = DMA_done(job_id);

    assert(res_done == job_id);
    assert(count_jobs(r.done) == 0);
    assert(count_jobs(r.free) == MAX_JOBS); 
    printf("    OK : Job libéré, retour dans la liste FREE.\n\n");

    printf("=== TOUS LES TESTS UNITAIRES DU MAIN 1 SONT PASSÉS AVEC SUCCÈS ===\n");

    free(ts);
    free(tt);
    
    return 0;
}


