#include <stdlib.h>
#include "hda.h"

static inline void __free_labels_cell(void* l, __attribute__((unused))void* unused) {
    if (l) free(l);
}

void free_cell(struct cell* c) {
    if (!c) return;
    if (c->d0) vector_destroy(c->d0);
    if (c->d1) vector_destroy(c->d1);
    if (c->labels) {
        // not sure if copy or not for now...
        vector_forall(c->labels, __free_labels_cell, NULL);
        vector_destroy(c->labels);
    }
    free(c);
}

struct cell* init_cell(size_t dim) {
    struct cell* c = calloc(1, sizeof(*c));
    if (!c) return NULL;
    c->dim = dim;
    if (dim) {
        c->d0 = vector_new(sizeof(struct cell*), dim);
        c->d1 = vector_new(sizeof(struct cell*), dim);
        c->labels = vector_new(sizeof(char*), dim);
    }
    return c;
}

static inline void __free_cells_hda(void* c, __attribute__((unused))void* unused) {
    free_cell(c);
}

void free_hda(struct hda* hda, bool free_content) {
    if (!hda) return;
    if (hda->cells && free_content)
        vector_forall(hda->cells, __free_cells_hda, NULL);
    if (hda->cells)
        vector_destroy(hda->cells);
    if (hda->final)
        vector_destroy(hda->final);
    if (hda->initial)
        vector_destroy(hda->initial);
    free(hda);
}

struct hda* init_hda(void) {
    struct hda* hda = malloc(sizeof(*hda));
    if (!hda) return NULL;
    hda->cells = vector_new(sizeof(struct cell*), 0);
    hda->initial = vector_new(sizeof(struct cell*), 0);
    hda->final = vector_new(sizeof(struct cell*), 0);
    if (!hda->cells || !hda->initial || !hda->final) {
        free_hda(hda, false);
        return NULL;
    }
    return hda;
}
