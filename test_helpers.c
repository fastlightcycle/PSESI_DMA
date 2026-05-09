#include "test_helpers.h"
#include <stdio.h>

couple_tab liste_verif[NBTESTS];

int ajout_couple(void* src, void* trg, size_t size, size_t test_num) {
    size_t byte_size_src = sizeof(src);
    size_t byte_size_trg = sizeof(trg);
    if(byte_size_src != byte_size_trg) {
        fprintf(stderr, "Tailles des deux tableaux pas égales");
        return -1;
    }

    couple_tab c;
    c.src = src;
    c.trg = trg;
    c.size = byte_size_trg;

    liste_verif[test_num] = c;

    return 0;
}

/**
 * Verrouille toutes les listes pour pouvoir lire l'état de `racine` 
 * Attention à l'ordre pour éviter les deadlocks si utilisé ailleurs (??)
 */
void dt_lock_all(void){
    pthread_mutex_lock(&r.free_lck);
    pthread_mutex_lock(&r.pending_lck);
    pthread_mutex_lock(&r.done_lck);
}

/**
 * Déverrouille les listes de racine 
 */
void dt_unlock_all(void){
    pthread_mutex_unlock(&r.done_lck);
    pthread_mutex_unlock(&r.pending_lck);
    pthread_mutex_unlock(&r.free_lck);
}

/**
 * Compte le nombre de jobs dans une liste.
 */
size_t dt_list_size(job *tete){
    size_t taille = 0;
    job *courant = tete;
    while (courant != NULL) {
        taille++;
        courant = courant->next;
    }
    return taille;
}

/**
 * Cherche un élément dans la liste
 */
int dt_is_present(job *tete, job *elem){
    if (!elem) return 0;
    job *courant = tete;
    while (courant != NULL) {
        if (courant == elem) return 1;
        courant = courant->next;
    }
    return 0;
}

/**
 * Vérifie si la liste est bien formée 
 * Utilise l'algorithme de détection de cycle (Lièvre et Tortue)
 * pour éviter les boucles infinies causées par des mauvais pointeurs next
 */
int dt_validate_integrated(job *tete){
    if(!tete) return 1; 

    job *t = tete;
    job *l = tete->next;

    while (l != NULL && l->next!= NULL ){
        if(t == l) return 0; // liste mal formée
        t = t->next;
        l = l->next->next;
    }
    return 1;
}

/**
 * Compte tous les jobs présents dans la racine 
 */
int dt_job_count(void){
    int total = 0;
    // On verrouille par précaution même si on est souvent en condition de test bloquée
    dt_lock_all();
    total += dt_list_size(r.free);
    total += dt_list_size(r.pending);
    total += dt_list_size(r.done);
    dt_unlock_all();
    return total;
}

/**
 * Vérifie que le pointeur correspond bien à un index du tableau global statique. 
 */
int dma_test_job_is_from_pool(job *j) {
    // On vérifie que l'adresse est bien comprise dans les limites du tableau
    if (j >= &jobs[0] && j < &jobs[MAX_JOBS]) {
        // Optionnel : s'assurer que c'est bien aligné sur le début d'une structure job
        // size_t offset = (size_t)j - (size_t)&jobs[0];
        // if (offset % sizeof(job) == 0) return 1;
        return 1;
    }
    return 0;
}

/**
 * Remet le sustème DMA à son état initial. 
* Très pratique à appeler dans le setUp() ou le tearDown() de tes tests ! (???) 
*/
void dma_test_reset_root_state(void) {
    dma_test_lock_all();
    
    r.free = NULL;
    r.pending = NULL;
    r.done = NULL;

    // Réinitialise tout le tableau dans la liste free
    for(int i = 0; i < MAX_JOBS; i++) {
        jobs[i].status = FREE;
        jobs[i].src = NULL;
        jobs[i].trg = NULL;
        jobs[i].prec = NULL;
        
        // Chaînage standard pour add_free
        jobs[i].next = r.free;
        r.free = &jobs[i];
    }
    
    dma_test_unlock_all();
}

void dt_init_tab(tab* t, int start_value) {
    if (!t) return;
    int* tab = (int*)t->stt;
    for (size_t i = 0; i < t->size; i++) {
        tab[i] = start_value + i;
    }
}

int dt_verify_data(tab* src, tab* trg) {
    if (!src || !trg || src->size != trg->size) return 0;
    
    int* src_array = (int*)src->stt;
    int* trg_array = (int*)trg->stt;
    
    for (size_t i = 0; i < src->size; i++) {
        if (src_array[i] != trg_array[i]) {
            printf("Mismatch at index %zu: Expected %d, Got %d\n", i, src_array[i], trg_array[i]);
            return 0; // Erreur de copie
        }
    }
    return 1; // Copie parfaite
}

#include <unistd.h> // usleep

int dt_wait_for_job_timeout(job* j, int timeout_ms) {
    int waited = 0;
    int step = 10; // vérifie toutes les 10 ms

    while (waited < timeout_ms) {
        // Lecture sécurisée du statut (pour éviter les data races)
        dma_test_lock_all();
        dma_test_unlock_all();

        if (j->status == DONE) {
            return 1; // Succès
        }

        usleep(step * 1000); // usleep prend des microsecondes
        waited += step;
    }
    printf("Timeout: %d ms\n", timeout_ms);
    return 0; // Échec
}

int dt_job_position(job *tete, job *elem) {
    int pos = 0;
    job *curr = tete;
    while (curr != NULL) {
        if (curr == elem) return pos;
        curr = curr->next;
        pos++;
    }
    return -1;
}