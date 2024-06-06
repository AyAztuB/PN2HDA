#include <stdio.h>
#include <string.h>

#include "logger.h"
#include "petri_nets.h"
#include "command_line.h"
#include "hda.h"

static void __xmlGenericErrorFunc (__attribute__((unused))void *ctx, __attribute__((unused))const char *msg, ...) { }

static void internal_help(const char* program) {
        char buff[1000] = { 0 };
        snprintf(buff, 1000, "Usage: %s [OPTIONS] FILE\n\twith FILE the pnml file to load (mandatory)\n", program);
        display_help(buff);
}

int main(int argc, char** argv) {
    xmlSetGenericErrorFunc(NULL, __xmlGenericErrorFunc);
    xmlThrDefSetGenericErrorFunc(NULL, __xmlGenericErrorFunc);

    add_argument("help", 'h', "display help message", true, (arg_default_value){ .is_set = false });
#ifndef NOLOG
    add_argument("logs", 0, "whether to display logs on stdout: YES|NO (default: YES)", false, (arg_default_value){ .value = "YES" });
    add_argument("log_date", 0, "whether to display date in stdout logs: YES|NO (default: NO)", false, (arg_default_value){ .value = "NO" });
#ifdef __linux__
    add_argument("log_threads", 0, "whether to display the thread id in the stdout logs: YES|NO (default: NO)", false, (arg_default_value){ .value = "NO" });
#endif // __linux__
    add_argument("log_file", 'f', "to specify a file to store logs (can be in addition of stdout logs)", false, (arg_default_value){ .value = NULL });
#endif // NOLOG
    add_argument("print_pn", 0, "use the petri net pretty print", true, (arg_default_value){ .is_set = false });

    if (argc == 1 || !parse_command_line(argc-1, argv) || is_flag_set("help")) {
        internal_help(argv[0]);
        return -1;
    }

#ifndef NOLOG
    struct logger_options l = (struct logger_options) {
        .output_logs = strcmp(get_argument_value("logs"), "YES") == 0,
        .show_date = strcmp(get_argument_value("log_date"), "YES") == 0,
#ifdef __linux__
        .show_thread_id = strcmp(get_argument_value("log_threads"), "YES") == 0,
#endif // __linux__
    };
    logger_set_options(l);
    const char* file_log = get_argument_value("log_file");
    if (file_log) {
        if (!logger_set_outfile(file_log))
            LOG(ERROR, "Unable to open log file `%s': skipping error", file_log);
    }
#endif // NOLOG

    xmlDocPtr document = xmlParseFile(argv[argc-1]);
    if (!document) {
        LOG(ERROR, "Cannot parse xml file `%s'", argv[argc-1]);
        return -1;
    }

    struct petri_net* net = parse_xml_file(xmlDocGetRootElement(document));
    xmlFreeDoc(document);

    if (!net) {
        LOG(ERROR, "Unable to get P/T net from file `%s'", argv[argc-1]);
        return -1;
    }

    if (is_flag_set("print_pn"))
        pn_pretty_print(net);

    struct hda* hda = conversion(net);

    struct cell** cells = vector_to_array(hda->cells);
    printf("cells:\n");
    for (size_t i = 0; i < vector_length(hda->cells); i++) {
        if (i) printf(",\n");
        printf("%zu(%p): dim=%zu", i, (void*)(cells[i]), cells[i]->dim);
        if (cells[i]->dim) {
            printf(":\t[");
            char** labels = vector_to_array(cells[i]->labels);
            for (size_t k = 0; k < vector_length(cells[i]->labels); k++) {
                if (k) printf(", ");
                printf("%s", labels[k]);
            }
            printf("]; d0: [");
            struct cell** d0 = vector_to_array(cells[i]->d0);
            for (size_t k = 0; k < vector_length(cells[i]->d0); k++) {
                if (k) printf(", ");
                printf("%p", (void*)d0[k]);
            }
            printf("]; d1: [");
            struct cell** d1 = vector_to_array(cells[i]->d1);
            for (size_t k = 0; k < vector_length(cells[i]->d1); k++) {
                if (k) printf(", ");
                printf("%p", (void*)d1[k]);
            }
            printf("]");
        }
    }
    printf("\n");

    // TODO: TO CONTINUE
    LOG(WARNING, "%s", "Features not implemented...");
    petri_net_destroy(net);
    free_hda(hda, true);
    LOG(INFO, "%s", "End of the program");


    free_argument_parser();
#ifndef NOLOG
    logger_close_outfile();
#endif // NOLOG

    return 0;
}
