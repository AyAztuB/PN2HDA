#include <stdlib.h>
#include "hashtbl.h"
#include "hda.h"

__attribute__((unused)) static inline void __free_labels_cell(void* l, __attribute__((unused))void* unused) {
    if (l && *(char**)l) free(*(char**)l);
}

void free_cell(struct cell* c) {
    if (!c) return;
    if (c->d0) vector_destroy(c->d0);
    if (c->d1) vector_destroy(c->d1);
    if (c->labels) {
        // not sure if copy or not for now...
        // vector_forall(c->labels, __free_labels_cell, NULL);
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
    } else {
        // d0 represent the incomming edges of our vertex and d1 the outgoing
        // up to 2 of each
        c->d0 = vector_new(sizeof(struct cell*), 2);
        c->d1 = vector_new(sizeof(struct cell*), 2);
    }
    return c;
}

static inline void __free_cells_hda(void* c, __attribute__((unused))void* unused) {
    if (c) free_cell(*((struct cell**)c));
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

static size_t _cmp_cell_ptr(const void* c1, const void* c2) {
    return c1 != c2;
}

static size_t _hash_cell_ptr(const void* key) {
    return (size_t)key << 1;
}

static struct hashtbl* init_printer(struct hda* hda) {
    struct hashtbl* out;
    HASHTBL_NEW(out, void*, size_t, .hash_func = _hash_cell_ptr, .cmp_func = _cmp_cell_ptr,);
    if (!out) return NULL;
    struct cell** cells = vector_to_array(hda->cells);
    size_t dim = 0, idx = 0;
    while (idx < vector_length(hda->cells)) {
        for (size_t i = 0; i < vector_length(hda->cells); i++) {
            if (cells[i]->dim == dim) {
                hashtbl_add(out, cells[i], (void*)idx, true);
                idx++;
            }
        }
        dim++;
    }
    return out;
}

void print_hda(struct hda* hda, FILE* out) {
    struct hashtbl* nb = init_printer(hda);
    if (!nb) return;
    struct cell** cells = vector_to_array(hda->cells);
    fprintf(out, "cells:\n");
    size_t dim = 0, idx = 0;
    while (idx < vector_length(hda->cells)) {
        for (size_t i = 0; i < vector_length(hda->cells); i++) {
            if (cells[i]->dim != dim) continue;
            if (i) fprintf(out, ",\n");
            fprintf(out, "%zu: dim=%zu", idx, dim);
            idx++;
            if (dim) {
                fprintf(out, ":\t[");
                char** labels = vector_to_array(cells[i]->labels);
                for (size_t k = 0; k < vector_length(cells[i]->labels); k++) {
                    if (k) fprintf(out, ", ");
                    fprintf(out, "%s", labels[k]);
                }
                fprintf(out, "]; d0: [");
                struct cell** d0 = vector_to_array(cells[i]->d0);
                for (size_t k = 0; k < vector_length(cells[i]->d0); k++) {
                    if (k) fprintf(out, ", ");
                    fprintf(out, "%zu", (size_t)hashtbl_find(nb, (void*)d0[k]).value);
                }
                fprintf(out, "]; d1: [");
                struct cell** d1 = vector_to_array(cells[i]->d1);
                for (size_t k = 0; k < vector_length(cells[i]->d1); k++) {
                    if (k) fprintf(out, ", ");
                    fprintf(out, "%zu", (size_t)hashtbl_find(nb, (void*)d1[k]).value);
                }
                fprintf(out, "]");
            }
        }
        dim++;
    }
    fprintf(out, "\n");
    hashtbl_destroy(nb);
}
