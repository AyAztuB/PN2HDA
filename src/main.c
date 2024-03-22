#include <stdio.h>
#include <string.h>
#include <libxml/parser.h>

#include "logger.h"

int main(int argc, char** argv) {
    if (argc != 2) {
        fprintf(stderr, "usage: %s FILE\n\twith FILE the pnml file to load\n", argv[0]);
        char msg[100] = { 0 };
        sprintf(msg, "invalid number of argument to the program: expected 1 but got %d!", argc - 1);
        LOG(ERROR, msg);
        return -1;
    }

    xmlDocPtr document = xmlParseFile(argv[1]);
    if (!document) {
        char* msg = strdup("Cannot parse xml file `");
        msg = realloc(msg, (strlen(msg) + strlen(argv[1]) + 2));
        msg = strcat(msg, argv[1]);
        msg[strlen(msg)] = '\'';
        LOG(FATAL, msg);
        free(msg);
        return -1;
    }

    xmlFreeDoc(document);
    LOG(WARNING, "features not implemented...");
    LOG(INFO, "End of the program");

    return 0;
}
