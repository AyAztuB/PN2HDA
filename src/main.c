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
    add_argument("print_hda", 0, "print the output HDA in stdout", true, (arg_default_value){ .is_set = false });
    add_argument("output", 'o', "output file to store the HDA", false, (arg_default_value){ .value = "out.hda" });

    if (argc == 1 || !parse_command_line(argc-1, argv) || is_flag_set("help") || !strcmp(argv[argc-1], "-h") || !strcmp(argv[argc-1], "--help")) {
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
    LOG(INFO, "%s", "Conversion algorithm finished");

    if (is_flag_set("print_hda"))
        print_hda(hda, stdout);

    const char* outFile = get_argument_value("output");
    if (outFile) {
        FILE* out = fopen(outFile, "w");
        if (!out) {
            LOG(ERROR, "Cannot open output file `%s'", outFile);
        } else {
            print_hda(hda, out);
            fclose(out);
        }
    }

    petri_net_destroy(net);
    free_hda(hda, true);
    LOG(INFO, "%s", "End of the program");


    free_argument_parser();
#ifndef NOLOG
    logger_close_outfile();
#endif // NOLOG

    return 0;
}
