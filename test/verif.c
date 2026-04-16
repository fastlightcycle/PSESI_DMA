#include <stdio.h>
#include "test_helpers.h"

int tab_verif(const void* src, const void* trg,size_t size){
    const char* s = (const char*)src;
    const char* t = (const char*)trg; 
    size_t i;
    for(i=0; i<size; i++){
        if(s[i] !=  t[i]) return -1;
    }

    return 0;
}

void tab_verif_finale(void *){
    for(size_t i = 0; i < NBTESTS; i++) {
        if( tab_verif(liste_verif[i].src, liste_verif[i].trg, liste_verif[i].size) ) {
            fprintf(stderr, "ERROR: Couple de tableaux %ld incorrect\n", i);
            return;
        }
    }

    printf("Verification réussie\n");
}


