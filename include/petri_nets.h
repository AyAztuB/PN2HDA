#ifndef PETRI_NETS_H
#define PETRI_NETS_H

#include "vector.h"

struct transition {
    char* label; // string
    Vector(int) previous; // vector<int> where int represent the index of the previous places
    Vector(int) next; // vector<int> like above but for next places
};

struct petri_net {
    Vector(struct transition*) transitions; // vector<struct transition*>
    Vector(int) marking; // vector<int> where each int represent the number of ressources at the given place
    // ie: place i has marking[i] ressources
};

#endif // PETRI_NETS_H
