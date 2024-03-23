#include <string.h>

#include "petri_nets.h"
#include "logger.h"
#include "hashtbl.h"

#define PNML_VERSION "http://www.pnml.org/version-2009/grammar/pnml"

/* PNML syntax (for P/T nets):
 *     { page }
 *         net
 *             { page }
 *                 { place [initialMarking] | transition | arc }
 * ie: we don't care about inscription for arcs and referencePlace/referenceTransition/toolspecific/graphics/name
 * if inscription or reference(Place | Transition) found => generate WARNING
 */

static struct petri_net* parse_petri_net(xmlNodePtr curr, struct petri_net* out, Hashtbl(char*, size_t) places, Hashtbl(char*, size_t) transitions) {
    if (!curr) return out;
    // TODO: TO CONTINUE
    LOG(WARNING, "%s", "Not implemented........");
    (void) curr;
    (void) places;
    (void) transitions;
    return out;
}

static inline void free_hashtbl_key(struct hashtbl_element elm, __attribute__((unused))void* unused) {
    free(elm.key);
}

static struct petri_net* search_petri_net(xmlNodePtr curr) {
    if (!curr)
        return NULL;

    if (!xmlStrcmp(curr->name, (const xmlChar*) "net")) {
        // TODO: assert type and have an id
        LOG(INFO, "%s", "NET found!");
        struct petri_net* res = petri_net_new();
        Hashtbl(char*, size_t) places = HASHTBL_NEW(char*, size_t, );
        Hashtbl(char*, size_t) transitions = HASHTBL_NEW(char*, size_t, );
        parse_petri_net(curr->children, res, places, transitions);
        hashtbl_forall(places, free_hashtbl_key, NULL), hashtbl_forall(transitions, free_hashtbl_key, NULL);
        return hashtbl_destroy(places), hashtbl_destroy(transitions), res;
    }

    struct petri_net* res = NULL;
    if (!xmlStrcmp(curr->name, (const xmlChar*) "page") && (res = search_petri_net(curr->children)))
        return res;

    return search_petri_net(curr->next);
}

struct petri_net* parse_xml_file(xmlNodePtr root) {
    if (!root) {
        LOG(ERROR, "%s", "XML file is empty!");
        return NULL;
    }
    if (xmlStrcmp(root->name, (const xmlChar*) "pnml")) {
        LOG(ERROR, "%s", "XML file is not a valid PNML file (missing <pnml> element at root)");
        return NULL;
    }

    bool found = false;
    for (xmlNs* ns = root->nsDef; ns; ns = ns->next) {
        if (ns->prefix == NULL) {
            if (xmlStrcmp(ns->href, (const xmlChar*) PNML_VERSION)) {
                LOG(ERROR, "PNML version not supported: expected `" PNML_VERSION "' but got `%s' as xml namespace definied in root element <pnml>", ns->href);
                return NULL;
            }
            found = true;
            break;
        }
    }
    if (!found) {
        LOG(ERROR, "%s", "Invalid PNML file: missing `xmlns' namespace definition in <pnml> root element");
        return NULL;
    }

    return search_petri_net(root->children);
}
