#include <string.h>
#include <ctype.h>

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

static char* skip_blank(char* str) {
    if (!str) return str;
    while (*str && isspace(*str)) str++;
    return str;
}

static void parse_place(xmlNodePtr curr, struct petri_net* net, Hashtbl(char*, size_t) places) {
    xmlChar* id = xmlGetProp(curr, (const xmlChar*) "id");
    if (!id) {
        LOG(ERROR, "%s", "Invalid transition: expected id attribute");
        return;
    }
    size_t val = 0;
    for (xmlNodePtr n = curr->children; n; n = n->next) {
        if (!xmlStrcmp(n->name, (const xmlChar*) "initialMarking")) {
            xmlNodePtr n2 = n->children;
            while (n2 && n2->type == XML_TEXT_NODE) n2 = n2->next;
            if (!n2 || xmlStrcmp(n2->name, (const xmlChar*) "text") || !n2->children) {
                LOG(WARNING, "Invalid initialMarking in place `%s'", id);
                continue;
            }
            xmlChar* save= xmlNodeListGetString(curr->doc, n2->children, 1);
            xmlChar* valStr = (xmlChar*) skip_blank((char*) save);
            char* rest = NULL;
            long v = 0;
            if (!valStr || (v = strtol((const char*) valStr, &rest, 10)) < 0 || (rest && *(skip_blank(rest)))) {
                LOG(WARNING, "Invalid initialMarking in place `%s': got `%s' but expected an unsigned integer", id, valStr);
                xmlFree(save);
                continue;
            }
            val = v;
            break;
        }
    }
    size_t p = vector_length(net->marking);
    vector_push(net->marking, &val);
    if (!hashtbl_add(places, id, (void*) p))
        LOG(ERROR, "The place `%s' cannot be added in the places hashtable...", id);
}

static void parse_transition(xmlNodePtr curr, struct petri_net* net, Hashtbl(char*, size_t) transitions) {
    xmlChar* id = xmlGetProp(curr, (const xmlChar*) "id");
    if (!id) {
        LOG(ERROR, "%s", "Invalid transition: expected id attribute");
        return;
    }
    xmlChar* name = NULL;
    for (xmlNodePtr n = curr->children; n; n = n->next) {
        if (!xmlStrcmp(n->name, (const xmlChar*) "name")) {
            xmlNodePtr n2 = n->children;
            while (n2 && n2->type == XML_TEXT_NODE) n2 = n2->next;
            if (!n2 || xmlStrcmp(n2->name, (const xmlChar*) "text") || !n2->children) {
                LOG(WARNING, "Invalid name in transition `%s'", id);
                continue;
            }
            name = xmlNodeListGetString(curr->doc, n2->children, 1);
            if (!name) {
                LOG(WARNING, "Invalid name in transition `%s': got null", id);
                xmlFree(name);
                name = NULL;
                continue;
            }
            break;
        }
    }
    size_t p = vector_length(net->transitions);
    if (!name)
        name = (xmlChar*) strdup((const char*) id);
    xmlChar* saveName = name;
    name = (xmlChar*)skip_blank((char*)name);
    for (int i = xmlStrlen(name) - 1; i >= 0 && isspace(name[i]); i--) name[i] = 0;
    vector_push(net->transitions, &(struct pn_transition*){ pn_transition_new((char*) name) });
    if (!hashtbl_add(transitions, id, (void*) p))
        LOG(ERROR, "The transition (id: `%s', name: `%s') cannot be added in the transitions hashtable...", id, name);
    free(saveName);
}

static void parse_arc(xmlNodePtr curr, struct petri_net* net, Hashtbl(char*, size_t) places, Hashtbl(char*, size_t) transitions) {
    xmlChar* id = xmlGetProp(curr, (const xmlChar*) "id");
    xmlChar* source = xmlGetProp(curr, (const xmlChar*) "source");
    xmlChar* target = xmlGetProp(curr, (const xmlChar*) "target");
    if (!id || !source || !target) {
        LOG(ERROR, "%s", "Invalid transition: expected id, source and target attributes");
        xmlFree(id);
        xmlFree(source);
        xmlFree(target);
        return;
    }
    for (xmlNodePtr n = curr->children; n; n = n->next) {
        if (!xmlStrcmp(n->name, (const xmlChar*) "inscription")) {
            LOG(WARNING, "Inscription found in arc `%s': inscriptions are ignored...", id);
        }
    }
    bool isFromPlace = true;
    struct hashtbl_element s = hashtbl_find(places, source);
    if (s.key == NULL && s.value == NULL) {
        isFromPlace = false;
        s = hashtbl_find(transitions, source);
    }
    struct hashtbl_element t;
    if (isFromPlace)
        t = hashtbl_find(transitions, target);
    else
        t = hashtbl_find(places, target);
    if ((s.key == NULL && s.value == NULL) || (t.key == NULL && t.value == NULL)) {
        LOG(ERROR, "Unable to find source or target of arc `%s'", id);
        xmlFree(id);
        xmlFree(source);
        xmlFree(target);
        return;
    }
    if (isFromPlace) {
        vector_push(((struct pn_transition**) vector_to_array(net->transitions))[(size_t) t.value]->preset, &(size_t){ (size_t) s.value });
    }
    else {
        vector_push(((struct pn_transition**) vector_to_array(net->transitions))[(size_t) s.value]->postset, &(size_t){ (size_t) t.value });
    }
    xmlFree(id);
    xmlFree(source);
    xmlFree(target);
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
