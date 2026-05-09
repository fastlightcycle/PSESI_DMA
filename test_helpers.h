#ifndef TEST_HELPERS_H
#define TEST_HELPERS_H

#include "dma.h"
#include "dma_sim.h"
#include <assert.h>
#include <stdio.h>


#define NBTESTS 3

typedef struct s_couple_tab{
    void *src;
    void *trg;
    size_t size; // taille en octet 
} couple_tab;

extern couple_tab liste_verif[NBTESTS];

// --- Prototypes des fonctions de test --- 
// je vais renommer dma_test par dt
int ajout_couple(void* src, void* trg, size_t size, size_t test_num);
void* tab_verif_finale(void *);

void dt_lock_all(void);
void dt_unlock_all(void);

size_t dt_list_size(job* tete); // job_list_count
int dt_is_present(job *tete, job *elem); // job_list_contains
int dt_validate_integrity(job *tete); // job_list_is_well_formed

int dt_job_count(void);
int dt_job_is_from_pool(job *j);
void dt_reset_root_state(void);

// --- Macros de tests

#define ASSERT_LIST_LEN(head, expected) \ 
    assert(dt_list_size(head) == (expected))

#define ASSERT_IN_LIST(head, jobptr) \ 
    assert(dt_is_present(head, jobptr) == 1)

#define ASSERT_NOT_IN_LIST(head, jobptr) \ 
    assert(dt_is_present(head, jobptr) == 0)

#define ASSERT_LIST_WELL_FORMED(head) \ 
    assert(dt_validate_integrity(head) == 1)

// Vérifie que la somme des jobs dans les 3 listes est bien égale à MAX_JOBS
#define ASSERT_DMA_COUNT() \ 
    assert(dma_test_job_count() == MAX_JOBS)

#define ASSERT_DMA_JOB_FAIL(call) \
    assert((uintptr_t)(call) == (uintptr_t)-1)

#define ASSERT_DMA_JOB_FULL(call) \
    assert((uintptr_t)(call) == 0)

#define ASSERT_DMA_JOB_OK(id) \
    assert((uintptr_t)(id) != 0 && (uintptr_t)(id) != (uintptr_t)-1)

#define ASSERT_JOB_FROM_POOL(id) \
    assert(dma_test_job_is_from_pool((job*)(id)) == 1)

    // Macro multi-lignes pour vérifier l'état complet d'un job
#define ASSERT_JOB_MATCHES(j, expected_src, expected_trg, expected_src_stp, expected_trg_stp, expected_size) \
    do { \
        assert((j)->status == PENDING); \
        assert((j)->src == (expected_src)); \
        assert((j)->trg == (expected_trg)); \
        assert((j)->src_stp == (expected_src_stp)); \
        assert((j)->trg_stp == (expected_trg_stp)); \
        assert((j)->size == (expected_size)); \
    } while(0)

#endif // TEST_HELPERS