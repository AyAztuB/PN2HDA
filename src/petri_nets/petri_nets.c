#include "petri_nets.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

struct pn_transition* pn_transition_new(const char* label) {
    struct pn_transition* t = malloc(sizeof(*t));
    if (!t) return NULL;
    t->label = strdup(label);
    if (!t->label) {
        free(t);
        return NULL;
    }
    t->preset = vector_new(sizeof(size_t), 16);
    if (!t->preset) {
        free(t->label);
        free(t);
        return NULL;
    }
    t->postset = vector_new(sizeof(size_t), 16);
    if (!t->postset) {
        vector_destroy(t->preset);
        free(t->label);
        free(t);
        return NULL;
    }
    return t;
}

void pn_transition_destroy(struct pn_transition* t) {
    if (!t) return;
    if (t->label)
        free(t->label);
    if (t->preset)
        vector_destroy(t->preset);
    if (t->postset)
        vector_destroy(t->postset);
    free(t);
}

struct petri_net* petri_net_new(void) {
    struct petri_net* pn = malloc(sizeof(*pn));
    if (!pn) return NULL;
    pn->marking = vector_new(sizeof(size_t), 0);
    if (!pn->marking) {
        free(pn);
        return NULL;
    }
    pn->transitions = vector_new(sizeof(struct pn_transition*), 0);
    if (!pn->transitions) {
        vector_destroy(pn->marking);
        free(pn);
        return NULL;
    }
    return pn;
}

static void free_pn_transition(void* t, __attribute__((unused))void* unused) {
    struct pn_transition** toto = t;
    pn_transition_destroy(*toto);
}

void petri_net_destroy(struct petri_net* pn) {
    if (!pn) return;
    if (pn->marking)
        vector_destroy(pn->marking);
    if (pn->transitions) {
        vector_forall(pn->transitions, free_pn_transition, NULL);
        vector_destroy(pn->transitions);
    }
    free(pn);
}

void pn_pretty_print(struct petri_net* pn) {
    printf("\nPetri Net:\n");
    if (!pn) {
        printf("(null)\n\n");
        return;
    }
    printf("Number of places: %zu, Number of transitions: %zu\n", vector_length(pn->marking), vector_length(pn->transitions));
    printf("Initial marking: ");
    size_t* initial_marking = vector_to_array(pn->marking);
    for (size_t i = 0; i < vector_length(pn->marking); i++) {
        if (i) printf(", ");
        printf("%zu", initial_marking[i]);
    }
    printf("\n");
    printf("Arcs:\n");
    struct pn_transition** transitions = vector_to_array(pn->transitions);
    for (size_t i = 0; i < vector_length(pn->transitions); i++) {
        printf("  transition '%s':\n    preset: ", transitions[i]->label);
        size_t* preset = vector_to_array(transitions[i]->preset);
        for (size_t k = 0; k < vector_length(transitions[i]->preset); k++) {
            if (k) printf(", ");
            printf("%zu", preset[k]);
        }
        printf("\n    postset: ");
        size_t* postset = vector_to_array(transitions[i]->postset);
        for (size_t k = 0; k < vector_length(transitions[i]->postset); k++) {
            if (k) printf(", ");
            printf("%zu", postset[k]);
        }
        printf(";\n");
    }
    printf("\n");
}
