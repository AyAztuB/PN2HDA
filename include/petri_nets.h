#ifndef PETRI_NETS_H
#define PETRI_NETS_H

#include <stddef.h>
#include <libxml/parser.h>

#include "vector.h"

struct pn_transition {
    char* label; // string
    Vector(size_t) preset; // vector<size_t> where int represent the index of the input places
    Vector(size_t) postset; // vector<size_t> like above but for output places
};

struct petri_net {
    Vector(struct pn_transition*) transitions; // vector<struct pn_transition*>
    Vector(size_t) marking; // vector<size_t> where each int represent the number of ressources at the given place
    // ie: place i has marking[i] ressources
};

struct pn_transition* pn_transition_new(const char* label);
void pn_transition_destroy(struct pn_transition* t);
struct petri_net* petri_net_new(void);
void petri_net_destroy(struct petri_net* pn);
struct petri_net* parse_xml_file(xmlNodePtr root);
void pn_pretty_print(struct petri_net* pn);

#endif // PETRI_NETS_H
