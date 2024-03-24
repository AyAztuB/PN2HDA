#include <string.h>

#include "petri_nets.h"
#include "logger.h"
#include "hashtbl.h"

#define PNML_VERSION "http://www.pnml.org/version-2009/grammar/pnml"
#define NET_TYPE "http://www.pnml.org/version-2009/grammar/ptnet"

/* PNML syntax (for P/T nets):
 *     { page }
 *         net
 *             { page }
 *                 { place [initialMarking] | transition | arc }
 * ie: we don't care about inscription for arcs and referencePlace/referenceTransition/toolspecific/graphics/name
 * if inscription or reference(Place | Transition) found => generate WARNING
 */

static void parse_place(xmlNodePtr curr, struct petri_net* net, Hashtbl(char*, size_t) places) {
    // TODO: TO CONTINUE
    (void) curr;
    (void) net;
    (void) places;
    LOG(WARNING, "%s", "Not implemented........");
}

static void parse_transition(xmlNodePtr curr, struct petri_net* net, Hashtbl(char*, size_t) transitions) {
    // TODO: TO CONTINUE
    (void) curr;
    (void) net;
    (void) transitions;
    LOG(WARNING, "%s", "Not implemented........");
}

static void parse_arc(xmlNodePtr curr, struct petri_net* net, Hashtbl(char*, size_t) places, Hashtbl(char*, size_t) transitions) {
    // TODO: TO CONTINUE
    (void) curr;
    (void) net;
    (void) places;
    (void) transitions;
    LOG(WARNING, "%s", "Not implemented........");
}

static struct petri_net* parse_petri_net(xmlNodePtr curr, struct petri_net* out, Hashtbl(char*, size_t) places, Hashtbl(char*, size_t) transitions) {
    if (!curr) return out;
    if (!xmlStrcmp(curr->name, (const xmlChar*) "page"))
        parse_petri_net(curr->children, out, places, transitions);
    else if (!xmlStrcmp(curr->name, (const xmlChar*) "place"))
        parse_place(curr, out, places);
    else if (!xmlStrcmp(curr->name, (const xmlChar*) "transition"))
        parse_transition(curr, out, transitions);
    else if (!xmlStrcmp(curr->name, (const xmlChar*) "arc"))
        parse_arc(curr, out, places, transitions);
    else if (!xmlStrcmp(curr->name, (const xmlChar*) "referencePlace") || !xmlStrcmp(curr->name, (const xmlChar*) "referenceTransition"))
        LOG(WARNING, "Node <%s> will be skipped....", curr->name);
    return parse_petri_net(curr->next, out, places, transitions);
}

static inline void free_hashtbl_key(struct hashtbl_element elm, __attribute__((unused))void* unused) {
    free(elm.key);
}

static struct petri_net* search_petri_net(xmlNodePtr curr) {
    if (!curr)
        return NULL;

    if (!xmlStrcmp(curr->name, (const xmlChar*) "net")) {
        xmlChar* type;
        if (!(type = xmlGetProp(curr, (const xmlChar*) "type")))
            LOG(WARNING, "%s", "NET node has no type attribute -> search for another net!");
        else if (xmlStrcmp(type, (const xmlChar*) NET_TYPE)) {
            LOG(WARNING, "Invalid NET type attribute: expected `" NET_TYPE "' but got `%s': search for another net!", type);
            xmlFree(type);
        }
        else {
            xmlFree(type);
            if (!(type = xmlGetProp(curr, (const xmlChar*) "id")))
                LOG(WARNING, "%s", "NET node has no id attribute -> try to take this net...");
            else {
                LOG(INFO, "NET found with id = `%s'", type);
                xmlFree(type);
            }
            struct petri_net* res = petri_net_new();
            Hashtbl(char*, size_t) places, *transitions;
            HASHTBL_NEW(places, char*, size_t, );
            HASHTBL_NEW(transitions, char*, size_t, );
            res = parse_petri_net(curr->children, res, places, transitions);
            hashtbl_forall(places, free_hashtbl_key, NULL), hashtbl_forall(transitions, free_hashtbl_key, NULL);
            return hashtbl_destroy(places), hashtbl_destroy(transitions), res;
        }
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
