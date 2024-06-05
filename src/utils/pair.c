#include <stdlib.h>

#include "pair.h"

struct pair new_pair(void* fst, void* snd) {
    return (struct pair) {
        .fst = fst,
        .snd = snd,
    };
}

void destroy_pair(struct pair p) {
    free(p.fst);
    free(p.snd);
}

void* pair_fst(struct pair p) {
    return p.fst;
}

void* pair_snd(struct pair p) {
    return p.snd;
}
