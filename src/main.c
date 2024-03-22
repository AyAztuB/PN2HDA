#include <stdio.h>
#include <string.h>

#include "logger.h"
#include "petri_nets.h"

int main(int argc, char** argv) {
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

    parse_xml_file(xmlDocGetRootElement(document));

    xmlFreeDoc(document);
    LOG(WARNING, "%s", "Features not implemented...");
    LOG(INFO, "%s", "End of the program");

    return 0;
}
