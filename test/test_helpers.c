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