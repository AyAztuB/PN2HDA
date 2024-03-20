#include <stdio.h>
#include <libxml/parser.h>

int main(int argc, char** argv) {
    if (argc != 2) {
        fprintf(stderr, "usage: %s FILE\n\twith FILE the pnml file to load\n", argv[0]);
        return -1;
    }

    xmlDocPtr document = xmlParseFile(argv[1]);
    if (!document) {
        fprintf(stderr, "Cannot parse xml file `%s'\n", argv[1]);
        return -1;
    }

    xmlFreeDoc(document);

    return 0;
}
