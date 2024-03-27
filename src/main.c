#include <stdio.h>
#include <string.h>

#include "logger.h"
#include "petri_nets.h"

static void __xmlGenericErrorFunc (__attribute__((unused))void *ctx, __attribute__((unused))const char *msg, ...) { }

int main(int argc, char** argv) {
    xmlSetGenericErrorFunc(NULL, __xmlGenericErrorFunc);
    xmlThrDefSetGenericErrorFunc(NULL, __xmlGenericErrorFunc);

    // TODO: parse command line arguments

    if (argc != 2) {
        LOG(ERROR, "Invalid number of argument: expected 1 but got %d!", argc - 1);
        fprintf(stderr, "Usage: %s FILE\n\twith FILE the pnml file to load\n", argv[0]);
        return -1;
    }

    xmlDocPtr document = xmlParseFile(argv[1]);
    if (!document) {
        LOG(ERROR, "Cannot parse xml file `%s'", argv[1]);
        return -1;
    }

    struct petri_net* net = parse_xml_file(xmlDocGetRootElement(document));
    xmlFreeDoc(document);

    if (!net) {
        LOG(ERROR, "Unable to get P/T net from file `%s'", argv[1]);
        return -1;
    }

    pn_pretty_print(net);

    // TODO: TO CONTINUE
    LOG(WARNING, "%s", "Features not implemented...");
    petri_net_destroy(net);
    LOG(INFO, "%s", "End of the program");

    return 0;
}
