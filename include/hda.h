#ifndef HDA_H
#define HDA_H

#include "vector.h"
#include "pair.h"
#include "petri_nets.h"

struct cell {
    size_t dim;
    Vector(struct cell*) d0; // Pair(struct cell*, char*) unstart label P.snd
    Vector(struct cell*) d1; // Pair(struct cell*, char*) finished label P.snd
    Vector(char*) labels;
    // Vector(Pair(struct cell*, char*)) up; //<< d+1 cells reachable from current with label P.snd starting
};

/*
cell->up is simple do create at the end of the hda:
for each c in hda->cells:
    for each d in c->d0:
        let l <- diff c->labels | d->labels // or d.snd if Pair
        add(c, l) in d->up // or in (d.fst)->up if Pair
    done
done
*/

struct hda {
    Vector(struct cell*) cells;
    Vector(struct cell*) initial;
    Vector(struct cell*) final;
};

void free_cell(struct cell* c);
struct cell* init_cell(size_t dim);
void free_hda(struct hda* hda, bool free_content);
struct hda* init_hda(void);

struct hda* conversion(struct petri_net* pn);

#endif // HDA_H
