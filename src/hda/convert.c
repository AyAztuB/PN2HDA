#include <stdlib.h>

#include "logger.h"
#include "hda.h"
#include "petri_nets.h"
#include "vector.h"
#include "hashtbl.h"
#include "pair.h"

static struct cell* _conversion(struct vector* m,
                                struct vector* transition_stack,
                                struct vector* pn, // transition part
                                struct hda* hda,
                                struct hashtbl* hashtbl) {
    size_t d = vector_length(transition_stack);
    struct cell* c = init_cell(d);
    if (!vector_push(hda->cells, &c)) {
        LOG(FATAL, "%s", "not enough memory");
        exit(1); // FIXME error handling
    }
    if (!hashtbl_add(hashtbl, m, c)) {
        LOG(FATAL, "%s", "not enough memory");
        exit(1); // FIXME error handling
    }
    for (size_t i = 0; i < vector_length(pn); i++) {
        struct vector* m2 = pn_start_transition(pn, m, i);
        // FIXME: Error handling in pn_start_transition not enough memory
        if (m2) {
            struct hashtbl_element e = hashtbl_find(hashtbl, m2);
            struct cell* c1 = e.value;
            if (e.key == NULL || e.value == NULL) {
                if (!vector_push(transition_stack, &i)) {
                    LOG(FATAL, "%s", "not enough memory");
                    exit(1); // FIXME error handling
                }
                c1 = _conversion(m2, transition_stack, pn, hda, hashtbl);
                vector_pop(transition_stack);
            }
            if (!vector_push(c1->d0, c)) {
                LOG(FATAL, "%s", "not enough memory");
                exit(1); // FIXME error handling
            }
            // TODO: add c.labels + pn.array[i].label in c1.labels
        }
    }
    if (d != 0) {
        size_t t = *(size_t*)(vector_pop(transition_stack));
        struct vector* m2 = pn_end_transition(pn, m, t);
        // FIXME: Error handling in pn_end_transition not enough memory
        if (!m2) {
            LOG(FATAL, "%s", "not enough memory");
            exit(1); // FIXME error handling
        }
        struct hashtbl_element e = hashtbl_find(hashtbl, m2);
        struct cell* c1 = e.value;
        if (e.key == NULL || e.value == NULL) {
            c1 = _conversion(m2, transition_stack, pn, hda, hashtbl);
            if (!vector_push(transition_stack, &t)) {
                LOG(FATAL, "%s", "not enough memory");
                exit(1); // FIXME error handling
            }
        }
        if (!vector_push(c->d1, c1)) {
            LOG(FATAL, "%s", "not enough memory");
            exit(1); // FIXME error handling
        }
        // TODO: add c1.labels + pn.array[t].label in c.labels
    }
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
    _conversion(pn->marking, t_stack, pn->transitions, out, h);
    vector_destroy(t_stack);
    hashtbl_forall(h, free_markings_hashtbl, NULL);
    hashtbl_destroy(h);
    return out;
}
