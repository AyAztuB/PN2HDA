#include <stddef.h>
#include <stdlib.h>

#include "logger.h"
#include "hda.h"
#include "petri_nets.h"
#include "vector.h"
#include "hashtbl.h"
#include "pair.h"

static struct cell* _conversion(struct vector* m,
                                struct vector* transition_stack, // stack of activated transition
                                struct vector* pn, // transition part
                                struct hda* hda,
                                struct hashtbl* hashtbl) {

    // dimension of the cell
    size_t d = vector_length(transition_stack);

    struct cell* c = init_cell(d);
    if (!c) {
        LOG(FATAL, "%s", "not enough memory");
        exit(1); // FIXME error handling
    }

    // add labels of currently activated transitions in the cell
    for (size_t i = 0; i < vector_length(transition_stack); i++) {
        // labels
        if (!vector_push(c->labels, &(((struct pn_transition**)vector_to_array(pn))[(((size_t*)vector_to_array(transition_stack))[i])]->label))) {
            LOG(FATAL, "%s", "not enough memory");
            exit(1); // FIXME error handling
        }
    }

    // add cell in the HDA
    if (!vector_push(hda->cells, &c)) {
        LOG(FATAL, "%s", "not enough memory");
        exit(1); // FIXME error handling
    }

    // add (marking, cell) in the hashtbl
    if (!hashtbl_add(hashtbl, m, c)) {
        LOG(FATAL, "%s", "not enough memory");
        exit(1); // FIXME error handling
    }

    // for all transition in the PN
    for (size_t i = 0; i < vector_length(pn); i++) {

        // try to start a transition (is_activable)
        struct vector* m2 = pn_start_transition(pn, m, i);
        // FIXME: Error handling in pn_start_transition not enough memory

        // if transition activable (and started successfully)
        if (m2) {
            // see if already known cell
            struct hashtbl_element e = hashtbl_find(hashtbl, m2);
            struct cell* c1 = e.value;
            if (e.key == NULL || e.value == NULL) {
                // if not already known, add transition i in stack + rec call + remove i from stack
                if (!vector_push(transition_stack, &i)) {
                    LOG(FATAL, "%s", "not enough memory");
                    exit(1); // FIXME error handling
                }
                c1 = _conversion(m2, transition_stack, pn, hda, hashtbl);
                vector_pop(transition_stack);
            } else {
                vector_destroy(m2);
            }

            // push actual cell as unstart of the reached one
            if (!vector_push(c1->d0, &c)) {
                LOG(FATAL, "%s", "not enough memory");
                exit(1); // FIXME error handling
            }
        }
    }

    // if we have some transition activated
    // iterate over the transition stack to terminate each one
    for (size_t k = 0; k < d; k++) {
        size_t t = ((size_t*)vector_to_array(transition_stack))[k];

        // copy the transition stack state without the ended transition
        struct vector* copy = vector_new(sizeof(size_t), vector_length(transition_stack)-1);
        for (size_t i = 0; i < d; i++) {
            if (i != k) vector_push(copy, &(((size_t*)vector_to_array(transition_stack))[i]));
        }

        // end that transition in the marking
        struct vector* m2 = pn_end_transition(pn, m, t);
        // FIXME: Error handling in pn_end_transition not enough memory
        if (!m2) {
            LOG(FATAL, "%s", "not enough memory");
            exit(1); // FIXME error handling
        }

        // see if reachable marking refer to a known cell
        struct hashtbl_element e = hashtbl_find(hashtbl, m2);
        struct cell* c1 = e.value;
        if (e.key == NULL || e.value == NULL) {
            // if not do a rec call
            c1 = _conversion(m2, copy, pn, hda, hashtbl);
        } else {
            vector_destroy(m2);
        }

        vector_destroy(copy);

        // add the reached cell in terminated of the current one
        if (!vector_push(c->d1, &c1)) {
            LOG(FATAL, "%s", "not enough memory");
            exit(1); // FIXME error handling
        }
    }

    // return the current cell
    return c;
}

static inline size_t marking_cmp(const void* m1, const void* m2) {
    return !is_same_marking((struct vector*)m1, (struct vector*)m2);
}

static inline void free_markings_hashtbl(struct hashtbl_element e, __attribute__((unused))void* unused) {
    vector_destroy(e.key);
}

struct hda* conversion(struct petri_net* pn) {
    struct hda* out = init_hda();
    Hashtbl(Vector(size_t), struct cell*) h;
    HASHTBL_NEW(h, Vector(size_t), struct cell*, .cmp_func = marking_cmp, .hash_func = marking_hash);
    Vector(size_t) t_stack = vector_new(sizeof(size_t), 0);
    if (!out || !t_stack || !h) {
        LOG(FATAL, "%s", "not enough memory");
        exit(1); // FIXME error handling
    }
    _conversion(marking_copy(pn->marking), t_stack, pn->transitions, out, h);
    vector_destroy(t_stack);
    hashtbl_forall(h, free_markings_hashtbl, NULL);
    hashtbl_destroy(h);
    return out;
}
