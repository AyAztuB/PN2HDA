#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "logger.h"
#include "hda.h"
#include "petri_nets.h"
#include "vector.h"
#include "hashtbl.h"
#include "pair.h"

struct _current_pn_state {
    struct vector* transition_stack;
    struct vector* pn_transitions;
    struct cell* current_cell;
    bool is_d0;
};

static bool _filter_hashtbl_elm(void* value, void* extra_args) {
    struct cell* c = value;
    struct _current_pn_state* pn_state = extra_args;
    // verifie cell found not full
    if (c->dim && pn_state->is_d0 && vector_length(c->d0) >= c->dim) return false;
    if (!pn_state->current_cell->dim && pn_state->is_d0 && vector_length(pn_state->current_cell->d1) >= 2) return false;
    if (!c->dim && !pn_state->is_d0 && vector_length(c->d0) >= 2) return false;
    // verifie c not in current_cell.(d0 U d1) and current_cell not in c.(d0 U d1)
    for (size_t i = 0; i < vector_length(c->d0); i++) {
        if (((struct cell**)vector_to_array(c->d0))[i] == pn_state->current_cell)
            return false;
    }
    for (size_t i = 0; i < vector_length(c->d1); i++) {
        if (((struct cell**)vector_to_array(c->d1))[i] == pn_state->current_cell)
            return false;
    }
    for (size_t i = 0; i < vector_length(pn_state->current_cell->d0); i++) {
        if (((struct cell**)vector_to_array(pn_state->current_cell->d0))[i] == c)
            return false;
    }
    for (size_t i = 0; i < vector_length(pn_state->current_cell->d1); i++) {
        if (((struct cell**)vector_to_array(pn_state->current_cell->d1))[i] == c)
            return false;
    }
    // verifie cell found has correct labels
    if (!c->labels)
        return vector_length(pn_state->transition_stack) == 0;
    if (vector_length(pn_state->transition_stack) != vector_length(c->labels))
        return false;
    for (size_t i = 0; i < vector_length(pn_state->transition_stack); i++) {
        long count = 1;
        const char* cur_label = ((struct pn_transition**)vector_to_array(pn_state->pn_transitions))[((size_t*)vector_to_array(pn_state->transition_stack))[i]]->label;
        for (size_t k = 0; k < vector_length(pn_state->transition_stack); k++) {
            if (k != i && !strcmp(cur_label,
                                  ((struct pn_transition**)vector_to_array(pn_state->pn_transitions))[((size_t*)vector_to_array(pn_state->transition_stack))[k]]->label))
                count++;
        }
        for (size_t k = 0; k < vector_length(c->labels); k++) {
            if (!strcmp(cur_label, ((char**)vector_to_array(c->labels))[k])) {
                if (!count) return false;
                count--;
            }
        }
        if (count) return false;
    }
    return true;
}

static struct cell* _conversion(struct vector* m,
                                struct vector* transition_stack, // stack of activated transition
                                struct vector* pn, // transition part
                                struct hda* hda,
                                struct hashtbl* hashtbl,
                                struct cell* S, struct cell* T) {

    // dimension of the cell
    size_t d = vector_length(transition_stack);

    struct cell* c = init_cell(d);
    if (!c) {
        LOG(FATAL, "%s", "not enough memory");
        exit(1); // FIXME error handling
    }

    if (S) {
        // push Start as unstart of the actual cell
        if (!vector_push(c->d0, &S)) {
            LOG(FATAL, "%s", "not enough memory");
            exit(1); // FIXME error handling
        }
    }
    if (S && !S->dim) {
        // push ougoing edge (current cell) in d1 of vertex S
        if (!vector_push(S->d1, &c)) {
            LOG(FATAL, "%s", "not enough memory");
            exit(1); // FIXME error handling
        }
    }
    if(T) {
        // add the actual cell in terminated of the Terminated cell
        if (!vector_push(T->d1, &c)) {
            LOG(FATAL, "%s", "not enough memory");
            exit(1); // FIXME error handling
        }
    }
    if (T && !d) {
        // push the incomming edge (T) in d0 of current vertex
        if (!vector_push(c->d0, &T)) {
            LOG(FATAL, "%s", "not enough memory");
            exit(1); // FIXME error handling
        }
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
    if (!hashtbl_add(hashtbl, m, c, false)) {
        LOG(FATAL, "%s", "not enough memory");
        exit(1); // FIXME error handling
    }

    // for all transition in the PN
    for (size_t i = 0; i < vector_length(pn); i++) {

        size_t is_activable = pn_transition_is_activable(pn, m, i);
        for (size_t j = 0; j < is_activable; j++) {
            // try to start a transition (is_activable)
            struct vector* m2 = pn_start_transition(pn, m, i);
            // FIXME: Error handling in pn_start_transition not enough memory

            // if transition activable (and started successfully)
            if (m2) {
                if (!vector_push(transition_stack, &i)) {
                    LOG(FATAL, "%s", "not enough memory");
                    exit(1); // FIXME error handling
                }
                // see if already known cell
                struct hashtbl_element e = hashtbl_find_filter(hashtbl, m2, _filter_hashtbl_elm, &(struct _current_pn_state) { transition_stack, pn, c, true });
                struct cell* c1 = e.value;
                if (e.key == NULL || e.value == NULL) {
                    // if not already known, add transition i in stack + rec call + remove i from stack
                    c1 = _conversion(m2, transition_stack, pn, hda, hashtbl, c, NULL);
                } else {
                    // push actual cell as unstart of the reached one
                    if (!vector_push(c1->d0, &c)) {
                        LOG(FATAL, "%s", "not enough memory");
                        exit(1); // FIXME error handling
                    }
                    if (!d) {
                        // push ougoing edge (current cell) in d1 of vertex S
                        if (!vector_push(c->d1, &c1)) {
                            LOG(FATAL, "%s", "not enough memory");
                            exit(1); // FIXME error handling
                        }
                    }
                    vector_destroy(m2);
                }
                vector_pop(transition_stack);
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
        struct hashtbl_element e = hashtbl_find_filter(hashtbl, m2, _filter_hashtbl_elm, &(struct _current_pn_state){ copy, pn, c, false });
        struct cell* c1 = e.value;
        if (e.key == NULL || e.value == NULL) {
            // if not do a rec call
            c1 = _conversion(m2, copy, pn, hda, hashtbl, NULL, c);
        } else {
            // add the reached cell in terminated of the current one
            if (!vector_push(c->d1, &c1)) {
                LOG(FATAL, "%s", "not enough memory");
                exit(1); // FIXME error handling
            }
            if (!c1->dim) {
                // push the incomming edge (T) in d0 of current vertex
                if (!vector_push(c1->d0, &c)) {
                    LOG(FATAL, "%s", "not enough memory");
                    exit(1); // FIXME error handling
                }
            }
            vector_destroy(m2);
        }

        vector_destroy(copy);
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
    _conversion(marking_copy(pn->marking), t_stack, pn->transitions, out, h, NULL, NULL);
    vector_destroy(t_stack);
    hashtbl_forall(h, free_markings_hashtbl, NULL);
    hashtbl_destroy(h);
    return out;
}
