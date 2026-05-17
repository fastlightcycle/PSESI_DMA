#include "test_helpers.h"
#include "dma.h"
#include <stdio.h>

int main() {
    DMA_init();
    printf("Test des dependances RAW, WAR, WAW\n\n");

    int taille = 50;

    // Buffers physiques
    int *buf_A = init_tab(taille, 10); // Valeurs : 10, 11, 12...
    int *buf_B = init_tab(taille, 50); // Valeurs : 50, 51, 52...
    int *buf_C = clear_tab(taille);    // Valeurs : 0, 0, 0...
    int *buf_D = clear_tab(taille);    // Valeurs : 0, 0, 0...

    // Création des tabs correspondants (flag mixte pour lecture/écriture)
    tab* tA = create_tab((uintptr_t*)buf_A, taille, 4, RED_RDY);
    tab* tB = create_tab((uintptr_t*)buf_B, taille, 4, RED_RDY | WRT_RDY);
    tab* tC = create_tab((uintptr_t*)buf_C, taille, 4, RED_RDY | WRT_RDY);
    tab* tD = create_tab((uintptr_t*)buf_D, taille, 4, WRT_RDY);

    // ==========================================
    // 1. Test RAW (Read After Write)
    // Job 1 écrit dans C, Job 2 lit C.
    // Résultat attendu : D doit contenir les valeurs de A.
    // ==========================================
    printf("Lancement RAW...\n");
    uintptr_t j1 = DMA_job(tA, tC, 1, 1, taille); // A -> C
    uintptr_t j2 = DMA_job(tC, tD, 1, 1, taille); // C -> D

    while(!DMA_done(j1));
    while(!DMA_done(j2));

    if(tab_verif_rapide(tA, tD) == 0) printf(">> RAW : SUCCES (D == A)\n\n");

    // ==========================================
    // 2. Test WAR (Write After Read)
    // Job 3 lit B, Job 4 écrase B.
    // Résultat attendu : C contient l'ancien B, B contient A.
    // ==========================================
    printf("Lancement WAR...\n");
    uintptr_t j3 = DMA_job(tB, tC, 1, 1, taille); // B -> C (Sauvegarde de B)
    uintptr_t j4 = DMA_job(tA, tB, 1, 1, taille); // A -> B (Ecrasement de B)

    while(!DMA_done(j3));
    while(!DMA_done(j4));

    // On vérifie que C a bien lu B avant qu'il ne soit écrasé
    int buf_B_original[50]; 
    for(int i=0; i<50; i++) buf_B_original[i] = 50 + i;
    tab* tB_orig = create_tab((uintptr_t*)buf_B_original, taille, 4, RED_RDY);

    if(tab_verif_rapide(tB_orig, tC) == 0 && tab_verif_rapide(tA, tB) == 0) 
        printf(">> WAR : SUCCES (C == ancien B, B == A)\n\n");

    // ==========================================
    // 3. Test WAW (Write After Write)
    // Job 5 écrit l'ancien B dans D, Job 6 écrit A dans D.
    // Résultat attendu : Le Job 6 gagne, D contient A.
    // ==========================================
    printf("Lancement WAW...\n");
    uintptr_t j5 = DMA_job(tB_orig, tD, 1, 1, taille); // B_orig -> D
    uintptr_t j6 = DMA_job(tA, tD, 1, 1, taille);      // A -> D

    while(!DMA_done(j5));
    while(!DMA_done(j6));

    if(tab_verif_rapide(tA, tD) == 0) 
        printf(">> WAW : SUCCES (D == A, l'ecriture J6 a ecrase J5)\n\n");


    DMA_cleanup();
    return 0;
}