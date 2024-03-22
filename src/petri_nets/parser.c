#include <string.h>

#include "petri_nets.h"
#include "logger.h"

#define PNML_VERSION "http://www.pnml.org/version-2009/grammar/pnml"

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
    LOG(INFO, "%s", "PNML file seems valid...");

    LOG(WARNING, "%s", "Not implemented........");
    return NULL;
}
